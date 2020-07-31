<?php
session_start();
if (!isset($_SESSION["admin"])) {
    die("<p>登录已过期或您无权访问</p><p><a href='/login.html'>点击重新登陆</a></p>");
}