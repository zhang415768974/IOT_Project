<?php
session_start();
require '../db_conn.php';
$action = $_GET['action'];
if ($action == 'login') {
    $loginname = mysqli_real_escape_string($conn, trim($_POST['loginname']));
    $password = md5(trim($_POST['password']));
    $sql = "select * from tb_admin where loginname='".$loginname."' and loginpasswd='".$password."' limit 1";
    $result = mysqli_query($conn, $sql);
    if (mysqli_num_rows($result) > 0) {
        $row = mysqli_fetch_assoc($result);
        $arr['status'] = 0;
        $arr['username'] = $row['username'];
        $_SESSION["admin"] = $row['username'];
        $arr['message'] = '登陆成功';
    } else {
        $arr['status'] = 1;
        $arr['message'] = '用户名或密码错误';
    }
    $conn->close();
    echo json_encode($arr);
} elseif ($action == 'logout') {
    $_SESSION["admin"] = null;
    unset($_SESSION);
    session_destroy();
    echo "退出成功";
}