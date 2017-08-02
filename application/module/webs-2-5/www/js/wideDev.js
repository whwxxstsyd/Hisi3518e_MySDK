var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

var alias_list = ["new camera", "new camera", "new camera"];
var host_list = ["192.168.1.11", "192.168.1.12", "192.168.1.13"];
var port_list = [7778, 7779, 7780];
var same_subnet_list = [1, 1, 1];

var d_alias = ["new camera", "new camera", "new camera", "", "", "", "", ""];
var d_host = ["192.168.1.11", "192.168.1.12", "192.168.1.13", "", "", "", "", ""];
var d_port = [7778, 7779, 7780, 0, 0, 0, 0, 0];
var d_username = ["admin", "admin", "admin", "", "", "", "", ""];
var d_password = ["admin", "admin", "admin", "", "", "", "", ""];

$(function(){
	/* 加载多画面参数 */
	initParams();
	/* 显示搜索到的设备 */
	add_search_result();	
	// 行与列
	var row,column;
	// 列表元素
	var $ul,$li;
	var ip = "127.0.0.1";
	var httpport = 80;
	// ===初始化设备参数===
	// 本机
	$("li#local_dev").html("<dl><dt><span>"+str_localdev+"</span></dt><hr class=\"cb\" /><dd><span>"+str_devalias+"</span>"+name+"</dd><dd><span>"+str_devip+"</span>"+ip+"</dd><dd><span>"+str_devport+"</span>"+httpport+"</dd></dl>");
	$("li#local_dev").addClass("isset");
	// 其他设备
	for(var i=0;i<8;i++){
		// 索引值对应的设备变量不为空
		if(!(d_host[i]==""||d_username[i]==""||d_password[i]=="")){
			row = parseInt((i+1)/3);
			column = parseInt((i+1)%3);
			$ul = $("ul.devs").eq(row);
			$li = $ul.children().eq(column);
			// 输出参数
			$li.html("<dl><dt><span>"+str_device+" "+i+"</span><span class=\"del\">X</span></dt><hr class=\"cb\" /><dd><span>"+str_devalias+"</span>"+d_alias[i]+"</dd><dd><span>"+str_devip+"</span>"+d_host[i]+"</dd><dd><span>"+str_devport+"</span>"+d_port[i]+"</dd></dl>");
			// 更改样式
			$li.addClass("isset");
		}
	}
	// 确认对话框
	$("#confirm_dialog").dialog({
		autoOpen:false,
		modal:true,
		draggable:false,
		resizable:false,
		title:str_dialogtitle_warn,
		buttons:[{
			text:str_btn_ok,
			click:function(){
				var $index = parseInt($("#confirm_dialog").attr("index"));
				// 列表元素索引
				var row = parseInt(($index+1)/3);
				var column = parseInt(($index+1)%3);
				var $ul = $("ul.devs").eq(row);
				var $li = $ul.children().eq(column);
				// 移除li下的子孙元素
				$li.empty();
				// 移除li的class属性
				$li.toggleClass();
				// 清除对应参数
				d_alias[$index] = "";
				d_host[$index] = "";
				d_port[$index] = 80;
				d_username[$index] = "";
				d_password[$index] = "";
				$(this).dialog("close");
			}
		},{
			text:str_btn_cancel,
			click:function(){
				$(this).dialog("close");
			}
		}]
	});
	// 设置对话框
	$("#modify_dialog").dialog({
		autoOpen:false,
		modal:true,
		draggable:false,
		resizable:false,
		width:400,
		title:str_dialogtitle_devSet,
		buttons:[{
			text:str_btn_ok,
			click:function(){
				// ===表单验证===
				// 设备名称
				if($("#d_alias").val()==""){
					tab_tips($("#d_alias"),str_error_devnameIsNull);
					$("#d_alias").focus();
					return;
				}
				// IP地址
				if($("#d_host").val()==""){
					tab_tips($("#d_host"),str_error_ipaddrIsNull);
					$("#d_host").focus();
					return;
				}
				if(!checkIP($("#d_host").val())){
					tab_tips($("#d_host"),str_error_ipaddrFormat);
					$("#d_host").focus();
					return;
				}
				// http端口
				if($("#d_port").val()==""){
					tab_tips($("#d_port"),str_error_portIsNull);
					$("#d_port").focus();
					return;
				}
				if(!checkport($("#d_port"),80)){
					return;
				}
				// 用户名
				if($("#d_username").val()==""){
					tab_tips($("#d_username"),str_error_usernameIsNull);
					$("#d_username").focus();
					return;
				}
				// 密码
				if($("#d_password").val()==""){
					tab_tips($("#d_password"),str_error_passwordIsNull);
					$("#d_password").focus();
					return;
				}
				// ===表单验证结束===
				// 变量索引
				var $index = parseInt($("#modify_frm").attr("index"));
				d_alias[$index] = $("#d_alias").val();
				d_host[$index] = $("#d_host").val();
				d_port[$index] = $("#d_port").val();
				d_username[$index] = $("#d_username").val();
				d_password[$index] = $("#d_password").val();
				// 列表元素索引
				row = parseInt(($index+1)/3);
				column = parseInt(($index+1)%3);
				$ul = $("ul.devs").eq(row);
				$li = $ul.children().eq(column);
				// 设置完，改变样式
				$li.addClass("issetting");
				// 在li上显示设置参数
				var i = $index;
				$li.html("<dl><dt><span>"+str_device+" "+i+"</span><span class=\"del\">X</span></dt><hr class=\"cb\" /><dd><span>"+str_devalias+"</span>"+d_alias[i]+"</dd><dd><span>"+str_devip+"</span>"+d_host[i]+"</dd><dd><span>"+str_devport+"</span>"+d_port[i]+"</dd></dl>");
				$("ul.devs li span.del").unbind("click");
				$("ul.devs li span.del").bind("click",function(event){
					var $ul = $(this).closest("ul.devs");
					var $li = $(this).closest("li");
					var $row = $ul.index();
					var $column = $li.index();
					var $index = $row*3+$column;
					$("#confirm_dialog").attr("index",$index-1);
					open_confirm(str_confirm_removeDev);
					event.stopPropagation();
				});
				$(this).dialog("close");
				// 完成设置后，自动保存
				setTimeout(function(){
					saveDevices();
				},500);
			}
		},{
			text:str_btn_cancel,
			click:function(){
				$(this).dialog("close");
			}
		}]
	}).show();
	// 点击，弹出设置对话
	$("ul.devs li:not(span.del)").click(function(){
		// 获取li元素所在的行数和列数
		var $row = $(this).parent("ul.devs").index();
		var $column = $(this).index();
		// 获取li元素的索引值
		var $index = $row*3+$column;
		// 如果是本机，不做设置
		if($index == 0){
			return;
		}
		// 设备变量索引值
		var i = $index-1;
		// 表单初始化
		$("#d_alias").val(d_alias[i]);
		$("#d_host").val(d_host[i]);
		$("#d_port").val(d_port[i]);
		$("#d_username").val(d_username[i]);
		$("#d_password").val(d_password[i]);
		// 标记当前表单对应的变量索引
		$("#modify_frm").attr("index",i);
		$("#modify_dialog").dialog("open");
		// 设备参数为空时的初始化
		if(!$("#d_alias").val()){
			$("#d_alias").val(str_device+" "+i);
			$("#d_host").focus();
		}
	});
	// 点击选项，自动填充表单
	$("#devSearch").on("change",function(){
		var $index = $("#devSearch").val();
		$("#d_alias").val(alias_list[$index]);
		$("#d_host").val(host_list[$index]);
		$("#d_port").val(port_list[$index]);
	});
	// 点击保存，提交所有设置
	$("#save").click(function(){
		saveDevices();
	});
	// 清空删除设备参数
	$("ul.devs li span.del").bind("click",function(event){
		var $ul = $(this).closest("ul.devs");
		var $li = $(this).closest("li");
		var $row = $ul.index();
		var $column = $li.index();
		var $index = $row*3+$column;
		$("#confirm_dialog").attr("index",$index-1);
		open_confirm(str_confirm_removeDev);
		event.stopPropagation();
	});
	/* ip地址验证 */
	function checkIP(ip){
		var expr=/^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/;
		if(ip.match(expr)==null){ 
			return false;
		}else{ 
			return true;
		}
	}
	/* 端口验证 */	
	function checkport($elm,special){
		if ($elm.val() == special||($elm.val() > 1024&&$elm.val() < 49151)){
			return true;
		}else{
			tab_tips($("#d_port"),str_error_portNum);
			$elm.focus();
			return false;
		}
	}
	function open_confirm(msg){
		$("#confirm_dialog").text(msg);
		$("#confirm_dialog").dialog("open");
	}
});	/* 初始化函数结束 */

function add_search_result() 
{
	var oOption = new Array();
	
	var cgiurl="/cgi-bin/cmd_proc.cgi?&cam_list&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				alias_list = alias_array;
				host_list = host_array;
				port_list = port_array;					
				same_subnet_list = same_subnet_array;						
			}
		});
	
	for (var i=0; i<host_list.length; i++)
	{
		oOption[i] = document.createElement("OPTION");	
		$("#devSearch").get(0).options.add(oOption[i]);
		if(same_subnet_list[i]){
			oOption[i].innerText = alias_list[i]+'('+host_list[i]+')';
			oOption[i].title = alias_list[i]+'('+host_list[i]+')';
			oOption[i].value = i;
		}
		else{
			oOption[i].style.color = 'gray';
			oOption[i].innerText = alias_list[i]+'('+str_subnet_nomatch+')';
		}		
	}	
}
// 保存设备
function saveDevices(){
	save_tips(str_saving);
	var cgiurl = "";
	var n;
	var wrong = 0;
	// 分别通过4次请求提交
	for(var i=0;i<4;i++){
		setTimeout("",500);
		cgiurl = "/cgi-bin/set_param.cgi?user=" + uname + "&pwd=" + upwd;
		for(var j=0;j<2;j++){
			n = i*2+j;
			cgiurl += "&multi_alias" + n + "=" + d_alias[n];
			cgiurl += "&multi_ip" + n + "=" + d_host[n];
			cgiurl += "&multi_port" + n + "=" + d_port[n];
			cgiurl += "&multi_user" + n + "=" +d_username[n];
			cgiurl += "&multi_pwd" + n + "=" +d_password[n];
			cgiurl += "&multi_alias" + (n+1) + "=" + d_alias[n+1];
			cgiurl += "&multi_ip" + (n+1) + "=" + d_host[n+1];
			cgiurl += "&multi_port" + (n+1) + "=" + d_port[n+1];
			cgiurl += "&multi_user" + (n+1) + "=" +d_username[n+1];
			cgiurl += "&multi_pwd" + (n+1) + "=" +d_password[n+1];			
		}
		cgiurl = encodeURI(cgiurl);
		$.get(cgiurl,function(data){
			if(data.indexOf("Error")!=-1){
				wrong++;
			}
		});
	}
	// 是否出错
	if(wrong > 0){
		save_tips(str_save_fail);
	}else{
		save_tips(str_save_succeed);
		setTimeout("document.location.reload();",1000);
	}
}

function WIDEDEV_MODEL(){
	this.getParams=function(){
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?&CFG_multi&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				d_alias = multi_alias;
				d_host = multi_ip;
				d_port = multi_port;					
				d_username = multi_user;						
				d_password = multi_pwd;
			}
		});
	}
	
	this.setParams=function(obj){
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
		cgiurl += "&snap_sd_interval=" + $("#SnapInterval").val();
	
		$.get(cgiurl);
	}
}

function initParams(){
	var recordObj=new WIDEDEV_MODEL();
	recordObj.getParams();	
}

function save(){
	new WIDEDEV_MODEL().setParams();
	save_tips(str_save_succeed);	
}