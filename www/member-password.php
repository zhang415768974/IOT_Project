<?php
require_once('app/session.php');
require_once('db_conn.php')
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
		<script type="text/javascript" src="./js/jquery.min.js"></script>
        <script type="text/javascript" src="./lib/layui/layui.js" charset="utf-8"></script>
        <script type="text/javascript" src="./js/xadmin.js"></script>
        <!-- 让IE8/9支持媒体查询，从而兼容栅格 -->
        <!--[if lt IE 9]>
            <script src="https://cdn.staticfile.org/html5shiv/r29/html5.min.js"></script>
            <script src="https://cdn.staticfile.org/respond.js/1.4.2/respond.min.js"></script>
        <![endif]--></head>
    
    <body>
        <div class="layui-fluid">
            <div class="layui-row">
                <form class="layui-form">
                    <div class="layui-form-item">
                        <label for="L_username" class="layui-form-label">客户名</label>
                        <div class="layui-input-inline">
						<?php
						$uid = (int)mysqli_real_escape_string($conn, trim($_GET['uid']));
						$sql = "select username from tb_customer where id=".$uid." limit 1";
						$result = mysqli_query($conn, $sql);
						if (mysqli_num_rows($result) > 0) {
							$row = mysqli_fetch_assoc($result);
							echo '<input type="text" id="L_username" name="username" disabled="" value="'.$row['username'].'" class="layui-input">';
							mysqli_free_result($result);
						} else {
							echo 'NULL';
						}
						?></div>
                    </div>
                    <div class="layui-form-item">
                        <label for="newpass" class="layui-form-label">
                            <span class="x-red">*</span>新密码</label>
                        <div class="layui-input-inline">
                            <input type="password" id="newpass" name="newpass" required="" lay-verify="required" autocomplete="off" class="layui-input"></div>
                        <div class="layui-form-mid layui-word-aux">6到16个字符</div></div>
                    <div class="layui-form-item">
                        <label for="repass" class="layui-form-label">
                            <span class="x-red">*</span>确认密码</label>
                        <div class="layui-input-inline">
                            <input type="password" id="repass" name="repass" required="" lay-verify="required" autocomplete="off" class="layui-input"></div>
                    </div>
                    <div class="layui-form-item">
                        <label class="layui-form-label"></label>
                        <button class="layui-btn" lay-filter="save" lay-submit="">确认</button></div>
                </form>
            </div>
        </div>
        <script>layui.use(['form', 'layer'],
            function() {
                $ = layui.jquery;
                var form = layui.form,
                layer = layui.layer;
                //监听提交
                form.on('submit(save)',
                function(data) {
                    if ($("input[name='newpass']").val() != $("input[name='repass']").val()) {
						layer.alert("两次密码输入不一致", {icon: 2});
						return false;
					}
					$.post("app/ajax.php?action=reset_memberpass", {uid: <?php echo $_GET["uid"]?>, newpass: $("input[name='newpass']").val()},
					function(result) {
						var result = jQuery.parseJSON(result);
						if (result.status == 0) {
							layer.alert("重置密码成功", {icon: 6}, function(){
								//关闭当前frame
								xadmin.close();
								// 可以对父窗口进行刷新 
								xadmin.father_reload();
							});
						} else {
							layer.msg(result.message);
						}
					})
                    return false;
                });

            });</script>
    </body>
</html>
<?php
$conn->close();