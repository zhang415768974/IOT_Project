<?php
require_once('app/session.php');
require_once("db_conn.php")
?>
<!DOCTYPE html>
<html class="x-admin-sm">
    <head>
        <meta charset="UTF-8">
        <title>欢迎页面-X-admin2.2</title>
        <meta name="renderer" content="webkit">
        <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
        <meta name="viewport" content="width=device-width,user-scalable=yes, minimum-scale=0.4, initial-scale=0.8,target-densitydpi=low-dpi" />
        <link rel="stylesheet" href="./css/font.css">
        <link rel="stylesheet" href="./css/xadmin.css">
        <script src="./lib/layui/layui.js" charset="utf-8"></script>
        <script type="text/javascript" src="./js/xadmin.js"></script>
        <!-- 让IE8/9支持媒体查询，从而兼容栅格 -->
        <!--[if lt IE 9]>
          <script src="https://cdn.staticfile.org/html5shiv/r29/html5.min.js"></script>
          <script src="https://cdn.staticfile.org/respond.js/1.4.2/respond.min.js"></script>
        <![endif]-->
    </head>
    <body>
        <div class="layui-fluid">
            <div class="layui-row layui-col-space15">
                <div class="layui-col-md12">
                    <div class="layui-card">
                        <div class="layui-card-body ">
                            <blockquote class="layui-elem-quote">欢迎管理员：
                                <span class="x-red"><?php echo $_SESSION["admin"] ?></span>！当前时间:<?php echo date("Y-m-d H:i:s");?>
                            </blockquote>
                        </div>
                    </div>
                </div>
                <div class="layui-col-md12">
                    <div class="layui-card">
                        <div class="layui-card-header">数据统计</div>
                        <div class="layui-card-body ">
                            <ul class="layui-row layui-col-space10 layui-this x-admin-carousel x-admin-backlog">
                                <li class="layui-col-md2 layui-col-xs6">
                                    <a href="javascript:;" class="x-admin-backlog-body">
                                        <h3>设备库存数</h3>
                                        <p>
                                            <cite>4</cite></p>
                                    </a>
                                </li>
                                <li class="layui-col-md2 layui-col-xs6">
                                    <a href="javascript:;" class="x-admin-backlog-body">
                                        <h3>总客户数</h3>
                                        <p>
                                            <cite>1</cite></p>
                                    </a>
                                </li>
                                <li class="layui-col-md2 layui-col-xs6">
                                    <a href="javascript:;" class="x-admin-backlog-body">
                                        <h3>激活设备数</h3>
                                        <p>
                                            <cite>4</cite></p>
                                    </a>
                                </li>
                            </ul>
                        </div>
                    </div>
                </div>
                <div class="layui-col-md12">
                    <div class="layui-card">
                        <div class="layui-card-header">系统信息</div>
                        <div class="layui-card-body ">
                            <table class="layui-table">
                                <tbody>
                                    <tr>
                                        <th>Iot物联网平台版本</th>
                                        <td>v2.0</td></tr>
                                    <tr>
                                        <th>服务器地址</th>
                                        <td><?php echo $_SERVER['SERVER_NAME'];?></td></tr>
                                    <tr>
                                        <th>操作系统</th>
                                        <td><?php echo php_uname();?></td></tr>
									<tr>
                                        <th>服务器CPU型号</th>
                                        <td><?php echo $_SERVER['PROCESSOR_IDENTIFIER'];?></td></tr>
                                    <tr>
                                        <th>运行环境</th>
                                        <td><?php echo $_SERVER['SERVER_SOFTWARE'];?></td></tr>
                                    <tr>
									<tr>
                                        <th>Zend版本</th>
                                        <td><?php echo Zend_Version();?></td></tr>
                                    <tr>
                                        <th>PHP版本</th>
                                        <td><?php echo PHP_VERSION;?></td></tr>
                                    <tr>
                                        <th>PHP运行方式</th>
                                        <td><?php echo php_sapi_name();?></td></tr>
                                    <tr>
                                        <th>MYSQL版本</th>
                                        <td>5.5.53</td></tr>
                                </tbody>
                            </table>
                        </div>
                    </div>
                </div>
                <style id="welcome_style"></style>
                <div class="layui-col-md12">
                    <blockquote class="layui-elem-quote layui-quote-nm">感谢layui,百度Echarts,jquery,本系统由x-admin提供技术支持。</blockquote></div>
            </div>
        </div>
        </div>
    </body>
</html>
<?php
$conn->close();