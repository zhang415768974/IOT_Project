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
                        <select id="members" name="members" class="valid">
                            <?php
                                mysqli_free_result($result);
                                $sql = "select id, username from tb_customer";
                                $result = mysqli_query($conn, $sql);
                                if (mysqli_num_rows($result) > 0) {
                                    while ($row = mysqli_fetch_assoc($result)) {
                                        echo '<option value='. $row['id'] . '>' . $row['username'] . '</option>';
                                    }
                                }
                            ?>
                        </select>
                    </div>
                  </div>
                  <div class="layui-form-item  layui-col-space5">
                      <label for="remark" class="layui-form-label">
                          <span class="x-red">*</span>生效时间
                      </label>
                      <div class="layui-inline">
                        <input class="layui-input" placeholder="开始时间" name="start" id="start">
                        </div>
                      <div class="layui-inline">
                      <input class="layui-input" placeholder="结束时间" name="end" id="end">
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
		layui.use('laydate',
        function() {
            var laydate = layui.laydate;
            //执行一个laydate实例
            laydate.render({
                elem: '#start' //指定元素
            });
            //执行一个laydate实例
            laydate.render({
                elem: '#end' //指定元素
            });
        });
		
		layui.use(['form', 'layer'],
            function() {
                $ = layui.jquery;
                var form = layui.form,
                layer = layui.layer;
                //监听提交
                form.on('submit(save)',
					function(data) {
						alert(1);
                        return false;
                    });
                });
            </script>
    </body>
</html>
<?php
mysqli_free_result($result);
$conn->close();