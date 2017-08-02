/* 按回车键提交表单 */
document.onkeydown=function(event)
{
	var e = event || window.event || arguments.callee.caller.arguments[0];    
	if(e && e.keyCode==13)
	{
		login();
	}
};

$(function(){
	//绑定当前语言
	$("#language").val(language_val);
	$("#language").change(function(){
		chgLanguagePack(this.value);
	});
	//用户名项获取焦点
	$("input[name=userName]").focus();
	//登录按钮绑定提交表单事件
	$("#loginBtn").bind("click",login);
});

function chgLanguagePack(language)
{
	setCookie("language", language, 365);	
}

/* 用户登陆 */
function login(){
	var $alert_msg=$("td.alertMsg");
	var $userName=$("input[name=userName]");
	
	//表单验证
	if($userName.val()==""){
		$alert_msg.text(str_userNameIsNull);
		$userName.focus();
		return;
	}
	

	var $pwd=$("input[name=userPwd]");
	
	if($pwd.val()==""){
		$alert_msg.text(str_pwdIsNull);
		$pwd.focus();
		return;
	}
	/*
	var userObj={
		"uname":$userName.val(),
		"upwd":$pwd.val(),
	}
	*/
	
	
	userObj = new Object();
	userObj.uname = $userName.val();
	userObj.upwd  = $pwd.val();		
	
	//提交表单
	if(loginImpl(userObj))
	{
		setCookie("username", Base64.encode($userName.val()), 365);
		setCookie("password", Base64.encode($pwd.val()), 365);
		setCookie("camera_model", "CAMERA_MODEL", 365);
		setCookie("oem_info", "OEM_INFO", 365);
		window.location.href="admin.html";
	}
	else
	{
		$alert_msg.text(str_userNameOrPwdError);
		$userName.val("");
		$pwd.val("");
		$alert_msg.val("");
		$userName.focus();		
	}
}

function loginImpl(obj){
	var status = false;
	
	
	var cgiurl = "/cgi-bin/get_param.cgi?user="+obj.uname+"&pwd="+obj.upwd;
	
	//document.write(obj.uname)
	//document.write(obj.upwd)
	//document.write(cgiurl)
	$.ajax(
	{
		url:cgiurl,
		async:false,
		cache:false,
		success: function(data)
		{
			//document.write(data)
			//eval(data);
			//if(user_level!="null")
			//{
				status=true
			//}
		}
	});
	return status;
}