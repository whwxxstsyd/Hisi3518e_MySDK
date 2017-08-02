var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function()
{
	get_value();	
	// 本地存储路径
	/*
	if(OS()=="Mac"){
		$("#localpath").hide();
	}else{
		$("#path_btn").attr("title",str_path);
		$("#path_btn").text(str_set);
		$("#localstore input").val(window.parent.document.getElementsByTagName("object")[0].GetRecordPath());
		$("#path_btn").click(function(){
			window.parent.document.getElementsByTagName("object")[0].setRecordPath();
			$("#localstore input").val(window.parent.document.getElementsByTagName("object")[0].GetRecordPath());
		});		
	}	
	*/	
});

function get_value(){
	var obj=getDeviceInfo();
	//MAC地址
	$("#macaddr").text(obj.macaddr);	
	//设备型号
	$("#devmodel").text(obj.devmodel);
	//设备名称
	$("#devname").text(obj.devname);
	//UID
	$("#uid").text(obj.uid);
	//硬件版本
	$("#hardver").text(obj.hardver);
	//软件版本
	$("#softver").text(obj.softver);
	//web版本
//	$("#webuiver").text(obj.webuiver);
	$("#webuiver").text(web_version);
	//系统开始运行时间
	$("#sysstartdate").text(obj.starttime);
	//SD卡状态
	if (obj.sdstatus == "none")
		$("#sdstatus").text(str_unintrusive);
	else
	{
		$("#sdstatus").text(obj.sdstatus);
		$("#formatconfirm_dialog").dialog({
			autoOpen:false,
			modal:true,
			draggable:false,
			resizable:false,
			title:str_dialogtitle_warn,
			buttons:[{
				text:str_btn_ok,
				click:function(){
					$(this).dialog("close");
					$("#coverlayer,#wait_content").show();
					$.get("/cgi-bin/cmd_proc.cgi?format_tf&user="+uname+"&pwd="+upwd,function(){
						window.location.reload();
					});
				}
			},{
				text:str_btn_cancel,
				click:function(){
					$(this).dialog("close");
				}
			}]
		});
		$("#format_btn").show().click(function(){
			if($("#formatconfirm_dialog").is(":hidden")){
				$("#formatconfirm_dialog").show();
			}
			$("#formatconfirm_dialog").dialog("open");
		});	
		$("#browse_btn").show();
		$("#remove_btn").show().click(function(){
			$.get("/cgi-bin/cmd_proc.cgi?remove_tf&user="+uname+"&pwd="+upwd,function(){
									window.location.reload();
								});
		});						
	}
		
	//UPNP状态
	if (obj.upnpstatus == "upnp_fail")
		$("#upnpstatus").text(str_failed);
	else if (obj.upnpstatus == "off")
		$("#upnpstatus").text(str_disable);
	else
		$("#upnpstatus").text(str_success);
	//DDNS状态
	if (obj.ddnsstatus == "update_ok")
		$("#ddnsstatus").text(str_ddns_update_ok);
	else if (obj.ddnsstatus == "off")
		$("#ddnsstatus").text(str_disable);		
	else
		$("#ddnsstatus").text(str_ddns_update_fail);		
	//NTP状态
	if (obj.ntpstatus == "ntp_sync_ok")
		$("#ntpstatus").text(str_ntp_update_ok);
	else if (obj.ntpstatus == "off")
		$("#ntpstatus").text(str_disable);				
	else
		$("#ntpstatus").text(str_ntp_update_fail);
}

function getDeviceInfo(){
	var res=null;
	var cgiurl="/cgi-bin/get_param.cgi?user="+uname+"&pwd="+upwd;
	cgiurl+="&CFG_ram&camera_type&camera_name&uid&hard_version&soft_version";
	cgiurl+="&start_time&eth_mac";
	$.ajax({
		url:cgiurl,
		async:false,
		cache:false,
		success: function(data){
			eval(data);
			res={
				"devmodel":camera_type,
				"devname":camera_name,
				"uid":uid,
				"hardver":hard_version,
				"softver":soft_version,
				"webuiver":"1.0.0",
				"starttime":start_time,
				"upnpstatus":upnp_status,
				"ddnsstatus":inner_status,
				"sdstatus":sd_capacity,
				"ntpstatus":ntp_status,
				"macaddr":eth_mac
			};
		}
	});
	return res;
}