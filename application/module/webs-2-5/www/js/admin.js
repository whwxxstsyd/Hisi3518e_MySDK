var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){
	$("#tab li").click(function(){
		$(this).addClass("current");
		$(this).siblings("li").removeClass("current");
		$("#loading").show();
		$("#ifectx").attr("src",$(this).attr("url"));
	});
	showUserInfo();
	$("#logOff").bind("click",logOff);
	
	if (with_playback == 0)
		$("#playback_page").hide();
		
	/* 用户权限管理 */				
	if (uname == "opt" || uname == "guest")
		$("#setting_page").hide();
});

function showUserInfo(){
	//显示当前登录用户信息
	var loginUser=Base64.decode(getCookie("username"));
	$("#loginUser").text(loginUser);
}

function logOff(){
	delCookie("username");
	delCookie("password");
	window.location.reload();
}

function ifeload(){
	var ifectx=document.getElementById("ifectx");
	if(ifectx.contentWindow.document.readyState == "complete"){
		setTimeout(function(){
			$("#loading").hide();
		},1000);
	}
}
