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
		<link rel="stylesheet" href="./basic/base.css"/>
		<link rel="stylesheet" href="./dist/css/calendar.min.css"/>
		<script type="text/javascript" src="./js/jquery.min.js"></script>
        <script type="text/javascript" src="./lib/layui/layui.js" charset="utf-8"></script>
        <script type="text/javascript" src="./js/xadmin.js"></script>
		<script type="text/javascript" src="./dist/js/calendar.min.js"></script>
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
                      <label for="machineid" class="layui-form-label">
                          <span class="x-red">*</span>机器码
                          <?php
							$id = (int)trim($_GET['id']);
							$sql = "select * from tb_device where id=".$id." limit 1";
							$result = mysqli_query($conn, $sql);
							if (mysqli_num_rows($result) > 0) {
								$row = mysqli_fetch_assoc($result);
							} else {
								die("设备数据不存在!");
							}
						?>
                      </label>
                      <div class="layui-input-inline">
                      <input type="text" id="machineid" name="machineid" disabled="" value="<?php echo $row['machineid'];?>" class="layui-input">
                      </div>
                  </div>
                  <div class="layui-form-item">
                      <label for="model" class="layui-form-label">
                          <span class="x-red">*</span>设备型号
                      </label>
                      <div class="layui-input-inline">
                          <input type="text" id="model" name="model" disabled="" value="<?php echo $row['model'];?>" class="layui-input">
                      </div>
                  </div>
                  <div class="layui-form-item">
                      <label for="remark" class="layui-form-label">
                          <span class="x-red">*</span>备注
                      </label>
                      <div class="layui-input-block">
                        <textarea  id="remark" name="remark" disabled="" class="layui-textarea"><?php echo $row['remark'];?></textarea>
                    </div>
                  </div>
                  <div class="layui-form-item">
                    <label for="remark" class="layui-form-label">
                          <span class="x-red">*</span>客户名
                    </label>
                    <div class="layui-input-inline">
						<input type="text" id="model" name="model" disabled="" value= 
                            <?php
                                mysqli_free_result($result);
                                $sql = "select username from tb_customer where id = ".$row['customerid']." limit 1";
                                $result = mysqli_query($conn, $sql);
                                if (mysqli_num_rows($result) > 0) {
									echo mysqli_fetch_assoc($result)['username'];
                                }
                            ?>
							class="layui-input">
                    </div>
                  </div>
                  <div class="layui-form-item  layui-col-space5">
                      <label for="remark" class="layui-form-label">
                          <span class="x-red">*</span>生效时间
                      </label>
                      <div class="layui-inline">
                        <input class="layui-input" placeholder="开始时间" name="start" id="start" required="" lay-verify="required" value="<?php echo $row['starttime'];?>">
                        </div>
                      <div class="layui-inline">
                      <input class="layui-input" placeholder="结束时间" name="end" id="end" required="" lay-verify="required" value="<?php echo $row['endtime'];?>">
                        </div>
                  </div>
                  <div class="layui-form-item">
                      <label for="L_repass" class="layui-form-label">
                      </label>
                      <button  class="layui-btn" lay-filter="save" lay-submit="">
                          确认激活
                      </button>
                  </div>
              </form>
            </div>
        </div>
        <script>
		xvDate({
			'targetId':'start',//时间写入对象的id
			'triggerId':['start'],//触发事件的对象id
			'alignId':'start',//日历对齐对象
			'format':'-',//时间格式 默认'YYYY-MM-DD HH:MM:SS'
			'min':'2020-08-01 00:00:00',//最大时间
			'max':'2099-12-31 23:59:59'//最小时间
        });
		xvDate({
			'targetId':'end',//时间写入对象的id
			'triggerId':['end'],//触发事件的对象id
			'alignId':'end',//日历对齐对象
			'format':'-',//时间格式 默认'YYYY-MM-DD HH:MM:SS'
			'min':'2020-08-01 00:00:00',//最大时间
			'max':'2099-12-31 23:59:59'//最小时间
        });
		layui.use(['form', 'layer'],
            function() {
                $ = layui.jquery;
                var form = layui.form,
                layer = layui.layer;
                //监听提交
                form.on('submit(save)',
					function(data) {
						$.post("app/ajax.php?action=updatedevice",{
							id: <?php echo $id;?>,
							start: $("input[name='start']").val(),
							end: $("input[name='end']").val()
							
						}, function(result){
							var result = jQuery.parseJSON(result);
							if (result.status == 0) {
								layer.alert("修改设备时间成功", {
								icon: 6
							},
							function() {
								//关闭当前frame
								xadmin.close();
								// 可以对父窗口进行刷新 
								xadmin.father_reload();
							});
							} else {
								layer.msg(result.message)
							}
						});
                        return false;
                    });
                });
            </script>
    </body>
</html>
<?php
mysqli_free_result($result);
$conn->close();