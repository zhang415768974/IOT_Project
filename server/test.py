#! /usr/bin/env python

import os
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

_1 = '''
    iot#1 : tick-req
    res#1 : tick-res
'''


class ErrCodeEnum(object):
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


class MainHandler(tornado.web.RequestHandler):
    def oauth(self, post_data):
        data_len = len(post_data)
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
    def get_device_status(self, machineid):
        error_code, response = ErrCodeEnum.success, ""
        if machineid not in globalcache:
            mysqlconn = globalcontext["mysql"]
            mysqlconn.ping()
            cursor = mysqlconn.cursor(cursor=pymysql.cursors.DictCursor)
            sql = '''select a.id, a.customerid, a.starttime, a.endtime, a.init_status, a.io_status, b.username, b.mobile from tb_device as a
            left join tb_customer as b
            on a.customerid = b.id
            where a.machineid = %(machineid)s limit 1'''
            rowcount = cursor.execute(sql, {"machineid": machineid})
            if rowcount == 0:
                error_code = ErrCodeEnum.device_notexists
                logging.warning("%s not exists" % machineid)
            else:
                result = cursor.fetchone()
                if not result or result["customerid"] == 0:
                    error_code = ErrCodeEnum.device_notregister
                    logging.warning("%s no register" % machineid)
                else:
                    if result["starttime"] and result["endtime"] and not result["starttime"] <= datetime.datetime.now() <= result["endtime"]:
                        error_code = ErrCodeEnum.expire
                        logging.warning("%s - %s expire" % (result["username"], machineid))
                    else:
                        globalcache[machineid] = result
                        logging.info("%s - %s online" % (result["username"], machineid))
            cursor.close()
            mysqlconn.close()
        if error_code == ErrCodeEnum.success:
            globalcache[machineid]["ttl"] = 2
            response = "res#1#%s" % globalcache[machineid]["io_status"]
        raise gen.Return([error_code, response])


    @gen.coroutine
    def set_device_status(self, machineid, value):
        error_code = ErrCodeEnum.success
        if machineid not in globalcache:
            error_code = ErrCodeEnum.device_notonline
        if globalcache[machineid]["io_status"] != value:
            mysqlconn = globalcontext["mysql"]
            mysqlconn.ping()
            cursor = mysqlconn.cursor(cursor=pymysql.cursors.DictCursor)
            rowcount = cursor.execute("update tb_device set io_status = %(io_status)s where machineid = %(machineid)s", {"io_status": value, "machineid": machineid})
            if rowcount > 0:
                mysqlconn.commit()
            cursor.close()
            mysqlconn.close()
            logging.info("%s - %s ioset %d" % (username, k, value))
        raise gen.Return([error_code, "res#2#%d" % value])


    @gen.coroutine
    def do_request(self):
        rawdata = self.get_body_argument("data", "")
        # logging.info("recv %s" % rawdata)
        error_code, response = ErrCodeEnum.success, ""
        if self.oauth(rawdata):
            self.set_header("Content-Type", "text/plain; charset=UTF-8")
            msg = rawdata.split("#")
            cmd, machineid, timestamp = int(msg[1]), msg[2], int(msg[3])
            if cmd == 1:
                error_code, response = yield self.get_device_status(machineid)
            elif cmd == 2:
                value = int(msg[4]) & 0xFF
                error_code, response = yield self.set_device_status(machineid, value)
            else:
                error_code = ErrCodeEnum.cmd_error
                # logging.error("cmd % error" % cmd)
        else:
            error_code = ErrCodeEnum.oauth_error
            # logging.error("oauth error")
        senddata = self.add_md5(response if error_code == ErrCodeEnum.success else "err#%d" % error_code)
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
    config.read('test.ini')
    for k, v in config.items():
        globalconfig[k] = {}
        for m, n in v.items():
            globalconfig[k][m] = n
    logging.info("load server config successfully!")

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
    ])

    server = globalconfig["server"]
    PeriodicCallback(watchdog, 5000).start()
    ioloop.IOLoop.instance().add_callback(watchdog)
    application.listen(server["port"], address=server["host"], xheaders=True)
    logging.info("server started at %s:%s" % (server["host"], server["port"]))
    ioloop.IOLoop.instance().start()