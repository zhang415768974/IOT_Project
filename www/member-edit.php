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
                        <label for="loginname" class="layui-form-label">
                            <span class="x-red">*</span>登录名</label>
                        <div class="layui-input-inline">
                        <?php
							$uid = (int)trim($_GET['uid']);
							$sql = "select * from tb_customer where id=".$uid." limit 1";
							$result = mysqli_query($conn, $sql);
							if (mysqli_num_rows($result) > 0) {
								$row = mysqli_fetch_assoc($result);
								echo '<input type="text" id="loginname" name="loginname" disabled="" value="'.$row['loginname'].'" class="layui-input">';
							} else {
								die("用户数据不存在!");
							}
						?>
						</div>
					</div>
                    <div class="layui-form-item">
                        <label for="username" class="layui-form-label">
                            <span class="x-red">*</span>客户名</label>
                        <div class="layui-input-inline">
                            <input type="text" id="username" name="username" required="" lay-verify="username" value="<?php echo $row["username"] ?>" class="layui-input"></div>
                    </div>
                    <div class="layui-form-item">
                        <label for="mobile" class="layui-form-label">
                            <span class="x-red">*</span>手机</label>
                        <div class="layui-input-inline">
                            <input type="text" id="mobile" name="mobile" required="" lay-verify="mobile" value="<?php echo $row["mobile"] ?>" class="layui-input"></div>
                        </div>
                    <div class="layui-form-item">
                        <label for="address" class="layui-form-label">
                            <span class="x-red">*</span>地址</label>
                        <div class="layui-input-inline">
                            <input type="text" id="address" name="address" required="" lay-verify="address" value="<?php echo $row["address"] ?>" class="layui-input"></div>
                    </div>
					<div class="layui-form-item">
                        <label for="remark" class="layui-form-label">
                            <span class="x-red">*</span>备注</label>
                        <div class="layui-input-inline">
                            <input type="text" id="remark" name="remark" required="" lay-verify="remark" value="<?php echo $row["remark"] ?>" class="layui-input"></div>
                    </div>
                    <div class="layui-form-item">
                        <label class="layui-form-label"></label>
                        <button class="layui-btn" lay-filter="save" lay-submit="">保存修改</button></div>
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
					$.post("app/ajax.php?action=edit_member", {
						uid: <?php echo $uid;?>,
						username: $("input[name='username']").val(),
						mobile: $("input[name='mobile']").val(),
						address: $("input[name='address']").val(),
						remark: $("input[name='remark']").val()},
					function(result) {
						var result = jQuery.parseJSON(result);
						if (result.status == 0) {
							layer.alert("修改客户资料成功", {icon: 6}, function(){
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
mysqli_free_result($result);
$conn->close();