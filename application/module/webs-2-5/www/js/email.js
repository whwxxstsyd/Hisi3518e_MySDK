var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){
	loadParams();
	$("#save").bind("click",save);
	$("#smtpTest").bind("click", smtp_test);
	
});

function smtp_test()
{
	test_tips(str_testing);
	var cgiurl="/cgi-bin/cmd_proc.cgi?email_test&user="+uname+"&pwd="+upwd;
	$.ajax({
				url:cgiurl,
				async:true,
				cache:false,
				success: function(data){
					eval(data);
					test_tips(email_test);
				}
			});
}

function EMAIL_MODEL(){
	this.getParams=function(){
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?CFG_email&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"sender":email_sender,
					"user":email_user,
					"pwd":email_pwd,
					"server":email_server,
					"port":email_port,
					"encryptType":email_mode,
					"recv0":email_recv0,
					"subject":email_subject,
					"context":email_context,
				};
			}
		});
		return res;
	}
	this.setParams=function(obj){
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
		cgiurl+="&email_sender="+obj.sender;		
		cgiurl+="&email_user="+obj.sender;
		cgiurl+="&email_pwd="+obj.pwd;
		cgiurl+="&email_server="+obj.server;
		cgiurl+="&email_port="+obj.port;
		cgiurl+="&email_mode="+obj.encryptType;
		cgiurl+="&email_recv0="+obj.recv0;
		cgiurl+="&email_subject="+obj.subject;
		cgiurl+="&email_context="+obj.context;
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
	var emailObj=new EMAIL_MODEL().getParams();
	$("#senderUserName").val(emailObj.sender);
	$("#senderPwd").val(emailObj.pwd);
	$("#smtpServer").val(emailObj.server);
	$("#smtpPort").val(emailObj.port);
	$("#encryptType").val(emailObj.encryptType);
	$("#receiverUserName").val(emailObj.recv0);
	$("#subject").val(emailObj.subject);
	$("#emailtext").val(emailObj.context);
}

function save(){
	var obj=new Object();
	obj.sender=$("#senderUserName").val();
	obj.pwd=$("#senderPwd").val();
	obj.server=$("#smtpServer").val();
	obj.port=$("#smtpPort").val();
	obj.encryptType=$("#encryptType").val();
	obj.recv0=$("#receiverUserName").val();
	obj.subject=$("#subject").val();
	obj.context=$("#emailtext").val();
	new EMAIL_MODEL().setParams(obj);
	save_tips(str_save_succeed);
}
