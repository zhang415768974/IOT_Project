<?php
require_once('app/session.php');
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
        <![endif]-->
    </head>
    <body>
        <div class="layui-fluid">
            <div class="layui-row">
                <form class="layui-form">
                    <div class="layui-form-item">
                        <label for="username" class="layui-form-label">
                            <span class="x-red">*</span>客户名</label>
                        <div class="layui-input-inline">
                            <input type="text" id="username" name="username" required="" lay-verify="required" class="layui-input"></div>
                        </div>
                    <div class="layui-form-item">
                        <label for="loginname" class="layui-form-label">
                            <span class="x-red">*</span>登录名</label>
                        <div class="layui-input-inline">
                            <input type="text" id="loginname" name="loginname" class="layui-input"></div>
							<div class="layui-form-mid layui-word-aux">
                            <span class="x-red">*</span>将会成为唯一的登入名,一经填写不可修改</div>
                    </div>
					<div class="layui-form-item">
                        <label for="mobile" class="layui-form-label">
                            <span class="x-red">*</span>联系方式</label>
                        <div class="layui-input-inline">
                            <input type="text" id="mobile" name="mobile" class="layui-input"></div>
                    </div>
					<div class="layui-form-item">
                        <label for="address" class="layui-form-label">
                            <span class="x-red">*</span>联系地址</label>
                        <div class="layui-input-inline">
                            <input type="text" id="address" name="address" class="layui-input"></div>
                    </div>
                    <div class="layui-form-item">
                        <label for="pass" class="layui-form-label">
                            <span class="x-red">*</span>密码</label>
                        <div class="layui-input-inline">
                            <input type="password" id="pass" name="pass" class="layui-input"></div>
                        <div class="layui-form-mid layui-word-aux">6到16个字符</div></div>
                    <div class="layui-form-item">
                        <label for="repass" class="layui-form-label">
                            <span class="x-red">*</span>确认密码</label>
                        <div class="layui-input-inline">
                            <input type="password" id="repass" name="repass" class="layui-input"></div>
                    </div>
					<div class="layui-form-item">
                        <label for="remark" class="layui-form-label">
                            备注</label>
                        <div class="layui-input-inline">
                            <input type="text" id="remark" name="remark" class="layui-input"></div>
                    </div>
                    <div class="layui-form-item">
                        <label class="layui-form-label"></label>
                        <button class="layui-btn" lay-filter="add" lay-submit="">增加</button></div>
                </form>
            </div>
        </div>
        <script>layui.use(['form', 'layer'],
			function() {
                var form = layui.form,
                layer = layui.layer;

                //监听提交
                form.on('submit(add)',
					function(data) {
						if ($("input[name='pass']").val() != $("input[name='repass']").val()) {
							layer.alert("两次密码输入不一致", {icon: 2});
							return false;
						}
						$.post("app/ajax.php?action=addcustomer", {
							username: $("input[name='username']").val(),
							loginname: $("input[name='loginname']").val(),
							mobile: $("input[name='mobile']").val(),
							address: $("input[name='address']").val(),
							pass: $("input[name='pass']").val(),
							remark: $("input[name='remark']").val()},
						function(result) {
                            var result = jQuery.parseJSON(result);
							if (result.status == 0) {
                                layer.alert("增加成功", {icon: 6}, function(){
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
					}
				)
			}
		);
		</script>
    </body>
</html>