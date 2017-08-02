var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function()			/* 页面加载时自动运行 */
{
	get_value();
	$("#save").bind("click", set_value);
});

function USER_MODEL()
{
	this.adminuser = "";
	this.adminpwd = "";
	this.optuser = "";
	this.optpwd = "";
	this.guestuser = "";
	this.guestpwd = "";
	
	this.getvalue = function()
	{
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?CFG_user&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"adminuser":admin_user,		//管理员用户
					"optuser":opt_user,				//操作者用户名
					"guestuser":guest_user,		//访客用户名
				};
			}
		});
		return res;	
	}	
	
	this.setvalue=function()
	{
		var res=null;
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
		
		cgiurl+="&admin_user="+this.adminuser;
		cgiurl+="&admin_pwd="+this.adminpwd;
		cgiurl+="&opt_user="+this.optuser;
		cgiurl+="&opt_pwd="+this.optpwd;
		cgiurl+="&guest_user="+this.guestuser;
		cgiurl+="&guest_pwd="+this.guestpwd;
		$.get(cgiurl);
	}	
}

function get_value(obj)
{
	var model = new USER_MODEL();
	var obj = model.getvalue();
	$("input[name=adminuser]").val(obj.adminuser);
	$("input[name=optuser]").val(obj.optuser);
	$("input[name=guestuser]").val(obj.guestuser);
}

function set_value(obj)
{
	var model = new USER_MODEL();
	if ($("input[name=adminpwd]").val() != $("input[name=adminrepwd]").val())
	{
		alert(str_error_pwdatypism);
		return;
	}
	if ($("input[name=optpwd]").val() != $("input[name=optrepwd]").val())
	{
		alert(str_error_pwdatypism);
		return;
	}
	if ($("input[name=guestpwd]").val() != $("input[name=guestrepwd]").val())
	{
		alert(str_error_pwdatypism);
		return;
	}
			
	model.adminuser=encodeURIComponent($("input[name=adminuser]").val());
	model.adminpwd=encodeURIComponent($("input[name=adminpwd]").val());
	model.optuser=encodeURIComponent($("input[name=optuser]").val());
	model.optpwd=encodeURIComponent($("input[name=optpwd]").val());
	model.guestuser=encodeURIComponent($("input[name=guestuser]").val());
	model.guestpwd=encodeURIComponent($("input[name=guestpwd]").val());
	model.setvalue();
	save_tips(str_save_succeed);	
}