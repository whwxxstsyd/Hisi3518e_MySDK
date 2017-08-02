var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

/* 先加载jquery */
document.write('<script type="text/javascript" src="js/jquery-ui-1.9.2.custom.min.js"></script>');
	
function CheckUpgrade(){
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?online_upgrade&online_version&online_message&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"online_upgrade":online_upgrade,
					"online_version":online_version,
					"online_message":online_message,
				};
			}
		});
		return res;
	}
	
	
$(function(){
	/* 提示对话框 */
	$("#warn_dialog").dialog({
		autoOpen:false,
		modal:true,
		draggable:false,
		resizable:false,
		title:str_dialogtitle_warn,
		open:function(){
			setTimeout('$("#warn_dialog").dialog("close")',2000);
		}
	});
	/* 确认对话框 */
	$("#confirm_dialog").dialog({
		autoOpen:false,
		modal:true,
		draggable:false,
		resizable:false,
		title:str_dialogtitle_warn,
		buttons:[{
			text:str_btn_ok,
			click:function(){
				$(this).dialog("close");
				/* 导入配置文件 */
				if($(this).text() == str_confirm_import){
					$("#restore_frm").submit();
				}
				/* 系统升级 */
				if($(this).text() == str_confirm_sysUpgrade){
					$("#upgrade_frm").submit();
					reboot("#progressbar",1000,window.location.port);
				}
				if ($(this).text() == str_confirm_autoUpgrade){
					var $url = "/cgi-bin/cmd_proc.cgi?sys_online_upgrade&user="+uname+"&pwd="+upwd;
					$.ajax({
						url:$url
					});						
					reboot("#progressbar",1000,window.location.port);					
				}
				/* 恢复出厂设置 */
				if($(this).text() == str_confirm_restoreFactory){
					var $url = "/cgi-bin/cmd_proc.cgi?sys_default&user="+uname+"&pwd="+upwd;
					$.ajax({
						url:$url
					});			
					reboot("#progressbar",300,window.location.port);
				}
				/* 设备重启 */
				if($(this).text() == str_confirm_devRestart){
					var $url = "/cgi-bin/cmd_proc.cgi?sys_reboot&user="+uname+"&pwd="+upwd;
					$.ajax({
						url:$url
					});
					reboot("#progressbar",300,window.location.port);
				}
			}
		},{
			text:str_btn_cancel,
			click:function(){
				$(this).dialog("close");
			}
		}]
	});
	
	/* 备份配置文件 */
	$("#backup").click(function(){
		parent.location.href = "cgi-bin/hi3510/backup.cgi";
	});
	/* 导入配置文件 */
	$("#import_btn").click(function(){
		if($("#restore_file").val() == ""){
			open_warn(str_warn_selectConfigFile);
			return;
		}
		open_confirm(str_confirm_import);
	})
	/* 系统升级 */
	$("#upgrade_btn").click(function(){
		if($("#upgrade_file").val() == ""){
			open_warn(str_warn_selectUpgradeFile);
			return;
		}
		open_confirm(str_confirm_sysUpgrade);
	});
	/* 在线升级 */
	$("#auto_upgrade_btn").click(function(){
		open_confirm(str_confirm_autoUpgrade);	
	});
	/* 恢复出厂设置 */
	$("#restore_btn").click(function(){
		open_confirm(str_confirm_restoreFactory);
	});
	/* 设备重启 */
	$("#restart_btn").click(function(){
		open_confirm(str_confirm_devRestart);
	});
	$("#restore_file").change(function(){
		var $val = $(this).val();
		$("#restore_filename").text($val);
	});
	$("#upgrade_file").change(function(){
		var $val = $(this).val();
		$("#upgrade_filename").text($val);
	});
	
	Obj = CheckUpgrade();
	if (Obj.online_upgrade > 0)
	{
		$("tr[name='online_upgrade_td']").show();
		$("#online_version").text(Obj.online_version);
		$("#online_message").text(Obj.online_message);	
	}
});		/* end function() */

function open_warn(msg){
	$("#warn_dialog").text(msg);
	$("#warn_dialog").dialog("open");
}
function open_confirm(msg){
	$("#confirm_dialog").text(msg);
	$("#confirm_dialog").dialog("open");
}
function progress(sec){
	var val = $("#progressbar").progressbar( "value" ) || 0;
	$("#progressbar").progressbar( "value", val + 1 );	
	if ( val < 180 ) {
		setTimeout(function(){
			progress(sec);
		}, sec);
	}
}