var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){
	loadParams();
	$("#save").bind("click",save);
	$("#ftpTest").bind("click",ftpTest);
});

function FTP_MODEL(){
	this.getParams=function(){
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?CFG_ftp&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"ftpServer":ftp_server,
					"ftpUser":ftp_user,
					"ftpPwd":ftp_pwd,
					"ftpPath":ftp_path,
					"ftpPort":ftp_port,
					"ftpMode":ftp_mode
				};
			}
		});
		return res;
	}
	this.setParams=function(obj){
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
		cgiurl+="&ftp_server="+obj.ftpServer;
		cgiurl+="&ftp_user="+obj.ftpUser;
		cgiurl+="&ftp_pwd="+obj.ftpPwd;
		cgiurl+="&ftp_path="+obj.ftpPath;
		cgiurl+="&ftp_port="+obj.ftpPort;
		cgiurl+="&ftp_mode="+obj.ftpMode;
		$.get(cgiurl);
	}
}


function loadParams(){
	// 开关控制
	$("ul#moreSet_switch li.switch_icon").click(function(){
		if($(this).hasClass("switch_icon_on")){
			$(this).removeClass("switch_icon_on");
			$(".moreSet_ctx").hide();
		}else{
			$(this).addClass("switch_icon_on");
			$(".moreSet_ctx").show();
		}
	});
	$("ul#pasvmode_switch li.switch_icon").click(function(){
		if($(this).hasClass("switch_icon_on")){
			$(this).removeClass("switch_icon_on");
		}else{
			$(this).addClass("switch_icon_on");
		}
	});
	var ftpObj=new FTP_MODEL().getParams();
	$("#ftpserver").val(ftpObj.ftpServer);
	$("#ftpusername").val(ftpObj.ftpUser);
	$("#ftpuserpwd").val(ftpObj.ftpPwd);
	$("#dirpath").val(ftpObj.ftpPath);
	$("#ftpport").val(ftpObj.ftpPort);
	if(ftpObj.ftpMode=="on"){
		$("ul#pasvmode_switch li.switch_icon").addClass("switch_icon_on");
	}
	
}

function ftpTest(){
	test_tips(str_testing);
	var cgiurl="/cgi-bin/cmd_proc.cgi?ftp_test&user="+uname+"&pwd="+upwd;
	$.ajax({
				url:cgiurl,
				async:true,
				cache:false,
				success: function(data){
					eval(data);
					test_tips(ftp_test);
				}
			});
}

function save(){
	var obj=new Object();
	obj.ftpServer=$("#ftpserver").val();
	obj.ftpUser=$("#ftpusername").val();
	obj.ftpPwd=$("#ftpuserpwd").val();
	obj.ftpPath=$("#dirpath").val();
	obj.ftpPort=$("#ftpport").val();
	obj.ftpMode=$("ul#pasvmode_switch li.switch_icon").hasClass("switch_icon_on")?"on":"off";
	new FTP_MODEL().setParams(obj);
	save_tips(str_save_succeed);	
}
