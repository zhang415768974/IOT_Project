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
        <script src="./lib/layui/layui.js" charset="utf-8"></script>
        <script type="text/javascript" src="./js/xadmin.js"></script>
        <!--[if lt IE 9]>
          <script src="https://cdn.staticfile.org/html5shiv/r29/html5.min.js"></script>
          <script src="https://cdn.staticfile.org/respond.js/1.4.2/respond.min.js"></script>
        <![endif]-->
    </head>
    <body>
        <div class="x-nav">
          <span class="layui-breadcrumb">
            <a href="">首页</a>
            <a href="">演示</a>
            <a>
              <cite>导航元素</cite></a>
          </span>
          <a class="layui-btn layui-btn-small" style="line-height:1.6em;margin-top:3px;float:right" onclick="location.reload()" title="刷新">
            <i class="layui-icon layui-icon-refresh" style="line-height:30px"></i></a>
        </div>
        <div class="layui-fluid">
            <div class="layui-row layui-col-space15">
                <div class="layui-col-md12">
                    <div class="layui-card">
                        <div class="layui-card-header">
                            <button class="layui-btn" onclick="xadmin.open('添加客户','./member-add.php',600,520)"><i class="layui-icon"></i>添加</button>
                        </div>
                        <div class="layui-card-body layui-table-body layui-table-main">
                            <table class="layui-table layui-form">
                                <thead>
                                  <tr>
                                    <th>ID</th>
                                    <th>客户名</th>
                                    <th>登录名</th>
                                    <th>手机</th>
                                    <th>地址</th>
                                    <th>注册时间</th>
                                    <th>备注</th>
                                    <th>操作</th></tr>
                                </thead>
                                <tbody>
<?php
$sql = "select * from tb_customer order by id desc limit 10";
$result = $conn->query($sql);
if ($result->num_rows > 0) { while ($row = $result->fetch_assoc()) {
?>
                                  <tr>
                                    <td><?php echo $row["id"]; ?></td>
                                    <td><?php echo $row["username"]; ?></td>
                                    <td><?php echo $row["loginname"]; ?></td>
                                    <td><?php echo $row["mobile"]; ?></td>
                                    <td><?php echo $row["address"]; ?></td>
                                    <td><?php echo $row["add_time"]; ?></td>
                                    <td><?php echo $row["remark"]; ?></td>
                                    <td class="td-manage">
                                      <a title="编辑"  onclick="xadmin.open('编辑','member-edit.php?uid=<?php echo $row["id"]; ?>',600,400)" href="javascript:;">
                                        <i class="layui-icon">&#xe642;</i>
                                      </a>
                                      <a onclick="xadmin.open('重置密码','member-password.php?uid=<?php echo $row["id"]; ?>',600,300)" title="重置密码" href="javascript:;">
                                        <i class="layui-icon">&#xe631;</i>
                                      </a>
                                      <a title="删除" onclick="member_del(this,<?php echo $row["id"]; ?>)" href="javascript:;">
                                        <i class="layui-icon">&#xe640;</i>
                                      </a>
                                    </td>
                                  </tr>
<?php }} ?>
                                </tbody>
                            </table>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </body>
<script>
function member_del(obj, uid) {
	layer.confirm('确认要删除吗？', function(index) {
		$.post("app/ajax.php?action=delcustomer", {userid: uid}, function(result){
			var result = jQuery.parseJSON(result);
			if (result.status == 0) {
				$(obj).parents("tr").remove();
				layer.msg('已删除!',{icon:1, time:1000});
			} else {
				layer.msg(result.message);
			}
		})
	});
}
</script>
</html>
<?php
$conn->close();
