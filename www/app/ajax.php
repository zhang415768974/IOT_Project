<?php
session_start();
require '../db_conn.php';
header('Content-type: text/plan;charset=utf-8'); 
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
	mysqli_free_result($result);
    $conn->close();
    echo json_encode($arr);
} elseif ($action == 'logout') {
    $_SESSION["admin"] = null;
    unset($_SESSION);
    session_destroy();
    echo "退出成功";
} elseif ($action == 'adddevice') {
	$machineid = mysqli_real_escape_string($conn, trim($_POST['machineid']));
	$model = mysqli_real_escape_string($conn, trim($_POST['model']));
	$remark = mysqli_real_escape_string($conn, trim($_POST['remark']));
	$sql = "select id from tb_device where machineid='".$machineid."' limit 1";
	$result = mysqli_query($conn, $sql);
	if (mysqli_num_rows($result) > 0) {
		$arr['status'] = 1;
		$arr['message'] = '机器码['.$machineid.']在库存中已有重复';
	} else {
		$sql = "insert into tb_device (machineid, model, remark) values ('".$machineid."','".$model."','".$remark."')";
		if (mysqli_query($conn, $sql)) {
			$arr['status'] = 0;
		} else {
			$arr['status'] = 2;
			$arr['message'] = '添加设备发生错误: ' . mysqli_error($conn);
		}
	}
	$conn->close();
	echo json_encode($arr);
} elseif ($action == 'deldevice') {
	$id = mysqli_real_escape_string($conn, trim($_POST['id']));
	$sql = "delete from tb_device where id=".$id;
	if (mysqli_query($conn, $sql)) {
		$arr['status'] = 0;
	} else {
		$arr['status'] = 1;
		$arr['message'] = '删除设备发生错误: ' . mysqli_error($conn);
	}
	$conn->close();
	echo json_encode($arr);
} elseif ($action == 'addcustomer') {
	$username = mysqli_real_escape_string($conn, trim($_POST['username']));
	$loginname = mysqli_real_escape_string($conn, trim($_POST['loginname']));
	$mobile = mysqli_real_escape_string($conn, trim($_POST['mobile']));
	$address = mysqli_real_escape_string($conn, trim($_POST['address']));
	$pass = md5(trim($_POST['pass']));
	$remark = mysqli_real_escape_string($conn, trim($_POST['remark']));
	$sql = "select id from tb_customer where loginname='".$loginname."' limit 1";
	$result = mysqli_query($conn, $sql);
	if (mysqli_num_rows($result) > 0) {
		$arr['status'] = 1;
		$arr['message'] = '登录名['.$loginname.']已有重复';
	} else {
		$sql = "insert into tb_customer (loginname, loginpasswd, username, address, mobile, remark) values ('".$loginname."','".$pass."','".$username."','".$address."','".$mobile."','".$remark."')";
		if (mysqli_query($conn, $sql)) {
			$arr['status'] = 0;
		} else {
			$arr['status'] = 2;
			$arr['message'] = '添加新客户发生错误: ' . mysqli_error($conn);
		}
	}
	$conn->close();
	echo json_encode($arr);
} elseif ($action == 'delcustomer') {
	$userid = (int)trim($_POST['userid']);
	$sql = "select id from tb_device where customerid=".$userid." limit 1";
	$result = mysqli_query($conn, $sql);
	if (mysqli_num_rows($result) > 0) { 
		$arr['status'] = 1;
		$arr['message'] = '该客户还有激活的设备,无法直接删除';
	} else {
		$sql = "delete from tb_customer where id=".$userid;
		if (mysqli_query($conn, $sql)) {
			$arr['status'] = 0;
		} else {
			$arr['status'] = 2;
			$arr['message'] = '删除客户发生错误: ' . mysqli_error($conn);
		}
	}
	$conn->close();
	echo json_encode($arr);
} elseif ($action == 'reset_memberpass') {
	$userid = (int)trim($_POST['uid']);
	$password = md5(trim($_POST['newpass']));
	$sql = "update tb_customer set loginpasswd='".$password."' where id=".$userid;
	if (mysqli_query($conn, $sql)) {
		$arr['status'] = 0;
	} else {
		$arr['status'] = 1;
		$arr['message'] = '重置密码发生错误: ' . mysqli_error($conn);
	}
	$conn->close();
	echo json_encode($arr);
} elseif ($action == 'edit_member') {
	$userid = (int)trim($_POST['uid']);
	$username = mysqli_real_escape_string($conn, trim($_POST['username']));
	$mobile = mysqli_real_escape_string($conn, trim($_POST['mobile']));
	$address = mysqli_real_escape_string($conn, trim($_POST['address']));
	$remark = mysqli_real_escape_string($conn, trim($_POST['remark']));
	$sql = "update tb_customer set username='".$username."',mobile='".$mobile."',address='".$address."',remark='".$remark."' where id=".$userid;
	if (mysqli_query($conn, $sql)) {
		$arr['status'] = 0;
	} else {
		$arr['status'] = 1;
		$arr['message'] = '修改客户资料发生错误: ' . mysqli_error($conn);
	}
	$conn->close();
	echo json_encode($arr);
} else {
	die("forbid");
}