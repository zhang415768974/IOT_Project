#! /usr/bin/env python

import os
import ctypes
import sys
import signal
import datetime
import logging
import time
import configparser

import hashlib
import pymysql
import random
import tornado
from tornado import web, gen, ioloop
from tornado.ioloop import PeriodicCallback
from time import time, localtime, strftime, sleep

globalconfig = {}
globalcontext = {}
globalcache = {}

logdirname = "temp"
MAX_PAGE_SIZE = 100

_1 = '''
    iot#1 : tick-req
    res#1 : tick-res
'''


class OpCodeEnum(object):
    success = 0
    oauth_error = 1
    cmd_error = 2
    device_notexists = 3
    device_notregister = 4
    expire = 5
    device_notonline = 6
    set_status_error = 7


def start_stats():
    '''事件日志输出初始化'''
    logger = logging.getLogger()
    logging.basicConfig(level=logging.INFO, format='%(asctime)s [%(levelname)s] %(message)s')
    rq = strftime('%Y%m%d', localtime(time()))
    if not os.path.exists(logdirname):
        os.makedirs(logdirname)
    logfile = os.path.join(os.path.dirname(__file__), logdirname, "%s.log" % rq)
    fh = logging.FileHandler(logfile, mode='a+')
    fh.setLevel(logging.INFO)
    fh.setFormatter(logging.Formatter('%(asctime)s [%(levelname)s] %(message)s'))
    fh.addFilter(NoAccessLoggingFilter())
    logger.addHandler(fh)
    logging.getLogger('tornado.access').addFilter(NoAccessLoggingFilter())


class NoAccessLoggingFilter(logging.Filter):
    def __init__(self, name='NoAccessLoggingFilter'):
        logging.Filter.__init__(self, name)

    def filter(self, record):
        return not record.getMessage()[:3] in ["200", "304", "404", "405"]


class BaseHandler(tornado.web.RequestHandler):
    def set_default_headers(self):
        self.set_header('Access-Control-Allow-Origin', '*')
        self.set_header('Access-Control-Allow-Methods', 'POST, OPTIONS')


class WebHandler(BaseHandler):
    @gen.coroutine
    def _get_onlinedevice(self, index):
        result = []
        for v in sorted(globalcache.values(), key=lambda _: _["customerid"], reverse=True)[index * MAX_PAGE_SIZE : (index + 1) * MAX_PAGE_SIZE]:
            result.append({
                "id": v["mid"],
                "cid": v["customerid"],
                "io_status": v["io_status"],
                "username": v["username"],
                "starttime": v["starttime"].strftime("%Y-%m-%d %H:%M:%S") if v["starttime"] else "-",
                "endtime": v["endtime"].strftime("%Y-%m-%d %H:%M:%S") if v["endtime"] else "-"
            })
        raise gen.Return(result)


    @gen.coroutine
    def _set_device_status(self, machineid, customerid, io_index, value):
        if machineid not in globalcache:
            raise gen.Return({"code": -1, "result": "device not online"})
        if io_index < 1 or io_index > 8:
            raise gen.Return({"code" :-2, "result": "io_index error"})
        if value not in (0, 1):
            raise gen.Return({"code": -2, "result": "value error"})

        if value:
            print("@@@", io_index, 1 << (io_index - 1))
            globalcache[machineid]["io_status"] |= 1 << (io_index - 1)
        else:
            globalcache[machineid]["io_status"] &= ~(1 << (io_index - 1))

        mysqlconn = globalcontext["mysql"]
        mysqlconn.ping()
        cursor = mysqlconn.cursor(cursor=pymysql.cursors.DictCursor)
        rowcount = cursor.execute("update tb_device set io_status = %(io_status)s where machineid = %(machineid)s and customerid = %(customerid)s",
         {"io_status": globalcache[machineid]["io_status"], "machineid": machineid, "customerid": customerid})
        if rowcount > 0:
            mysqlconn.commit()
        cursor.close()
        mysqlconn.close()
        logging.info("%s - %s set io_status %d" % (globalcache[machineid]["username"], machineid, globalcache[machineid]["io_status"]))
        raise gen.Return({"code": 0, "result": "ok"})


    @gen.coroutine
    def do_request(self):
        cmd = self.get_body_argument("cmd", "")
        response = {}
        if cmd == "get_onlinedevice":
            index = int(self.get_body_argument("index", 0))
            response = yield self._get_onlinedevice(index)
        elif cmd == "set_iostatus":
            machineid = self.get_body_argument("machineid", "")
            customerid = int(self.get_body_argument("customerid", 0))
            io_index = int(self.get_body_argument("io_index", 0))
            io_value = int(self.get_body_argument("io_value", 0))
            response = yield self._set_device_status(machineid, customerid, io_index, io_value)
        self.write(tornado.escape.json_encode(response))
        self.finish()


    @gen.coroutine
    def post(self):
        yield self.do_request()


    @gen.coroutine
    def options(self):
        self.set_status(204)
        self.finish()


class MainHandler(tornado.web.RequestHandler):
    def oauth(self, post_data):
        if not (post_data[:3] == "iot" and post_data[-3:] == "eof"):
            return False
        signature_str = "%s%s" % (post_data[:-36], globalconfig["server"]["secret_key"])
        token = hashlib.md5(signature_str.encode("utf8")).hexdigest()
        if token != post_data[-36:-4]:
            return False
        return True


    def add_md5(self, raw_data):
        signature_str = "%s#%s" % (raw_data, globalconfig["server"]["secret_key"])
        token = hashlib.md5(signature_str.encode("utf8")).hexdigest()
        return "%s#%s" % (raw_data, token)


    @gen.coroutine
    def get_device_status(self, machineid, customerid):
        result_code, response = OpCodeEnum.success, ""
        if machineid not in globalcache:
            mysqlconn = globalcontext["mysql"]
            mysqlconn.ping()
            cursor = mysqlconn.cursor(cursor=pymysql.cursors.DictCursor)
            sql = '''select a.id, a.customerid, a.starttime, a.endtime, a.io_status, b.username, b.mobile from tb_device as a
            left join tb_customer as b
            on a.customerid = b.id
            where a.machineid = %(machineid)s and a.customerid = %(customerid)s limit 1'''
            rowcount = cursor.execute(sql, {"machineid": machineid, "customerid": customerid})
            if rowcount == 0:
                result_code = OpCodeEnum.device_notexists
                logging.warning("%s not exists" % machineid)
            else:
                result = cursor.fetchone()
                if not result or result["customerid"] == 0:
                    result_code = OpCodeEnum.device_notregister
                    logging.warning("%s no register" % machineid)
                else:
                    if result["starttime"] and result["endtime"] and not result["starttime"] <= datetime.datetime.now() <= result["endtime"]:
                        result_code = OpCodeEnum.expire
                        logging.warning("%s - %s expire" % (result["username"], machineid))
                    else:
                        result["mid"] = machineid
                        globalcache[machineid] = result
                        logging.info("%s - %s online" % (result["username"], machineid))
            cursor.close()
            mysqlconn.close()
        if result_code == OpCodeEnum.success:
            globalcache[machineid]["ttl"] = 2
            response = "res#1#%s" % globalcache[machineid]["io_status"]
        raise gen.Return([result_code, response])


    @gen.coroutine
    def set_device_status(self, machineid, customerid, value):
        result_code = OpCodeEnum.success
        mysqlconn = globalcontext["mysql"]
        mysqlconn.ping()
        cursor = mysqlconn.cursor(cursor=pymysql.cursors.DictCursor)
        rowcount = cursor.execute("update tb_device set io_status = %(io_status)s where machineid = %(machineid)s and customerid = %(customerid)s", {"io_status": value, "machineid": machineid, "customerid": customerid})
        if rowcount > 0:
            mysqlconn.commit()
        cursor.close()
        mysqlconn.close()
        logging.info("%s ioset %d" % (customerid, value))
        raise gen.Return([result_code, "res#2#%d" % value])


    @gen.coroutine
    def do_request(self):
        rawdata = self.get_body_argument("data", "")
        if int(globalconfig["server"]["debug"]):
            logging.info("recv %s" % rawdata)
        result_code, response = OpCodeEnum.success, ""
        if self.oauth(rawdata):
            self.set_header("Content-Type", "text/plain; charset=UTF-8")
            msg = rawdata.split("#")
            cmd, machineid, customerid, timestamp = int(msg[1]), msg[2], int(msg[3]), int(msg[4])
            if customerid != 0:
                if cmd == 1:
                    result_code, response = yield self.get_device_status(machineid, customerid)
                elif cmd == 2:
                    value = int(msg[5]) & 0xFF
                    result_code, response = yield self.set_device_status(machineid, customerid, value)
                else:
                    result_code = OpCodeEnum.cmd_error
                    # logging.error("cmd % error" % cmd)
            else:
                result_code = OpCodeEnum.device_notregister
        else:
            result_code = OpCodeEnum.oauth_error
            # logging.error("oauth error")
        senddata = self.add_md5(response if result_code == OpCodeEnum.success else "err#%d" % result_code)
        # logging.info("send %s" % senddata)
        self.write(senddata)
        self.finish()


    @gen.coroutine
    def post(self):
        yield self.do_request()


@gen.coroutine
def watchdog():
    if not globalcache:
        return
    mysqlconn = globalcontext["mysql"]
    mysqlconn.ping()
    cursor = mysqlconn.cursor(cursor=pymysql.cursors.DictCursor)
    rowcount = cursor.execute("select machineid, starttime, endtime, io_status from tb_device where customerid != 0")
    if rowcount == 0:
        cursor.close()
        return
    result = cursor.fetchall()
    for m in result:
        k = m["machineid"]
        if k not in globalcache:
            continue
        obj = globalcache[k]
        username = globalcache[k]["username"]
        if m["starttime"] and m["endtime"] and not m["starttime"] <= datetime.datetime.now() <= m["endtime"]:
            logging.warning("%s - %s expire" % (username, k))
            del globalcache[k]
        else:
            if obj["ttl"] <= 0:
                logging.info("%s - %s offline" % (username, k))
                del globalcache[k]
            else:
                obj["ttl"] -= 1
                if obj["io_status"] != m["io_status"]:
                    obj["io_status"] = m["io_status"]
                    logging.info("%s - %s iochange" % (username, k))
        yield gen.sleep(0.001)
    cursor.close()
    mysqlconn.close()


def shutdown():
    '''Stop server and add a callback to stop I/O loop'''
    try:
        io_loop = ioloop.IOLoop.instance()
        logging.info('Will shutdown in 2 seconds ...')
        io_loop.add_timeout(time() + 1, io_loop.stop)
    except Exception as e:
        logging.exception(str(e))


def sig_handler(sig,frame):
    '''Catch signals and callback'''
    ioloop.IOLoop.instance().add_callback(shutdown)


if __name__ == "__main__":
    start_stats()
    config = configparser.ConfigParser()
    config.read('server.ini')
    for k, v in config.items():
        globalconfig[k] = {}
        for m, n in v.items():
            globalconfig[k][m] = n
    logging.info("load server config successfully!")
    ctypes.windll.kernel32.SetConsoleTitleW("华慧物联网IOT网关服务器[运行中...]")
    mysql = globalconfig["mysql"]
    try:
        globalcontext["mysql"] = pymysql.connect(host=mysql["host"], port=int(mysql["port"]), user=mysql["user"], passwd=mysql["passwd"], db=mysql["db"], charset=mysql["charset"])
        logging.info("connect mysql %s successfully!", globalcontext["mysql"].get_server_info())
    except Exception as e:
        logging.error(e)
        shutdown()

    random.seed(time())
    # add signal handler to stop server
    signal.signal(signal.SIGTERM, sig_handler)
    signal.signal(signal.SIGINT, sig_handler)

    application = tornado.web.Application([
        (r"/", MainHandler),
        (r"/restserver/handler", WebHandler)
    ])

    server = globalconfig["server"]
    PeriodicCallback(watchdog, 5000).start()
    ioloop.IOLoop.instance().add_callback(watchdog)
    application.listen(server["port"], address=server["host"], xheaders=True)
    logging.info("server started at %s:%s" % (server["host"], server["port"]))
    ioloop.IOLoop.instance().start()