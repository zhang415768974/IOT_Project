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
		<style>
		.io_open { color:green; font-size:bolder;}
		.io_close{ color:#cccccc;}
		</style>
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
                <i class="layui-icon layui-icon-refresh" style="line-height:30px"></i>
            </a>
        </div>
        <div class="layui-fluid">
            <div class="layui-row layui-col-space15">
                <div class="layui-col-md12">
                    <div class="layui-card">
                        <div class="layui-card-body ">
                            <table class="layui-table layui-form">
                                <thead>
                                    <tr>
                                        <th>ID</th>
										<th>机器码</th>
										<th>型号</th>
										<th>当前状态</th>
										<th>所属客户</th>
										<th>联系方式</th>
										<th>开始时间</th>
										<th>截止时间</th>
										<th>备注</th>
										<th>操作</th>
									</tr>
                                </thead>
                                <tbody>
<?php
$sql = "select a.id, a.machineid, a.model, a.starttime, a.endtime, a.remark, b.username, b.mobile from tb_device as a left join tb_customer as b on a.customerid = b.id where customerid != 0 order by id desc limit 10";
$result = $conn->query($sql);
if ($result->num_rows > 0) { while ($row = $result->fetch_assoc()) {
?>

<tr>
                                    <td><?php echo $row["id"]; ?></td>
                                    <td><?php echo $row["machineid"]; ?></td>
                                    <td><?php echo $row["model"]; ?></td>
									<td> - </td>
									<td><?php echo $row["username"]; ?></td>
									<td><?php echo $row["mobile"]; ?></td>
                                    <td><?php echo $row["starttime"]; ?></td>
									<td><?php echo $row["endtime"]; ?></td>
									<td><?php echo $row["remark"]; ?></td>
									<td class="td-manage">
                                      <a title="修改有效期"  onclick="xadmin.open('修改有效期','device-edit.php?id=<?php echo $row["id"]; ?>',600,480)" href="javascript:;">
                                        <i class="layui-icon">&#xe642;</i>
                                      </a>
									  <a title="释放设备" onclick="device_release(this,<?php echo $row["id"]; ?>)" href="javascript:;">
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
function device_release(obj, did) {
	layer.confirm('确认要释放设备吗？', function(index) {
		$.post("app/ajax.php?action=releasedevice", {id: did}, function(result){
			var result = jQuery.parseJSON(result);
			if (result.status == 0) {
				$(obj).parents("tr").remove();
				layer.msg('已释放设备!',{icon:1, time:1000});
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