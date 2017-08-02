document.write('<script type="text/javascript" src="js/jquery-1.9.1.min.js"></script>');

var web_version = "1.0.0.0";

/* ===================  cookie操作  ==================== */
/* 获取变量值 */
function getCookie(name){
	var strCookie=document.cookie;
    var arrCookie=strCookie.split('; ');
    for (var i=0;i<arrCookie.length;i++){
		var arr=arrCookie[i].split('=');
        if (arr[0]==name)
			return unescape(arr[1]);
    }
    return null;
}

/* 设置变量值 */
function setCookie(name,value,expirehours){
	var cookieString=name+'='+escape(value);
    if (expirehours>0){
		var date=new Date();
        date.setTime(date.getTime()+expirehours*3600*1000);
        cookieString=cookieString+'; expires='+date.toGMTString();
	}
    document.cookie=cookieString;
}

/* 检查Cookie */
function chkCookie(c_name) {
    var username = getCookie(c_name);
    if (username != null && username != "") 
	{
        return true;
    } 
	else 
	{
        return false;
    }
}
/* 删除Cookie */
function delCookie(name){
    var exp = new Date();
    exp.setTime(exp.getTime() - 1);
    var cval = getCookie(name);
    if (cval != null) document.cookie = name + "=" + cval + ";expires=" + exp.toGMTString();
}

/* =================== 获取浏览器版本 ==================== */
function GetBrowserVer() 
{
		/* 浏览器版本号函数 */
    var br = navigator.userAgent.toLowerCase();		
    var browserVer = (br.match(/.+(?:rv|it|ra|ie)[\/: ]([\d.]+)/) || [0, '0'])[1];
    var brower;
    /* js浏览器判断函数 */
    var browserName = navigator.userAgent.toLowerCase();

    if (/msie/i.test(browserName) && !/opera/.test(browserName)) 
        brower = "IE";
    else if (/firefox/i.test(browserName))
        brower = "Firefox";
    else if (/chrome/i.test(browserName) && /webkit/i.test(browserName) && /mozilla/i.test(browserName))
        brower = "Chrome";
    else if (/opera/i.test(browserName))
        brower = "Opera";
    else if (/webkit/i.test(browserName) && !(/chrome/i.test(browserName) && /webkit/i.test(browserName) && /mozilla/i.test(browserName)))
        brower = "Safari";
   	else if (/trident/i.test(browserName) && !/opera/.test(browserName))
        brower = "IE";
    else
        brower = "unKnow";

    return {brower: brower, ver: browserVer};			/* 返回对象 */
}

/* =================== 判断操作系统 ==================== */
function OS()
{
		if(navigator.userAgent.indexOf("Window") > 0)
				return "Windows";
		else if(navigator.userAgent.indexOf("Mac OS X") > 0)
				return "Mac";
		else if(navigator.userAgent.indexOf("Linux") > 0)
			  return "Linux";
		else
			 	return "NUll";
}

/* 切换语言 */
function language_chg(obj){
	var val = obj.value;
	setCookie("language",val,30);
	location.reload();
}
/* 切换分辨率 */
function resolution_chg(obj){
	var val = obj.value;
	setCookie("resolution",val,30);
	window.location.reload();
}

/* 信息提示 */
/* msg提示信息内容 */
function tips($evt,msg){
	var $tips=$evt.siblings("span.tips");
	if($tips.length==0){
		$evt.after('<span class="tips">'+msg+'</span>').siblings("span.tips").fadeOut(2500,function(){
			$(this).remove();
		});
	}
}
/* 表单验证 */
/* msg提示信息内容 */
function tab_tips($evt,msg){
	var $tips=$evt.siblings(".tips");
	if($tips.length==0){
		$evt.after('<div class="tips">'+msg+'</div>').siblings(".tips").fadeOut(2500,function(){
			$(this).remove();
		});
	}
}
/* 文本框验证 */
/* msg提示信息内容 */
function input_tips($evt,msg){
	var $tips=$evt.siblings("span.input_tips");
	if($tips.length==0){
		$evt.after('<span class="input_tips">'+msg+'</span>');
	}else{
		$tips.remove();
		$evt.after('<span class="input_tips">'+msg+'</span>');
	}
}
/* 保存数据 */
function dosubmit(cgiurl){
	cgiurl = encodeURI(cgiurl);
	save_tips(str_saving);
	$.get(cgiurl,function(data){
		if(data.indexOf("Error")==-1){
			save_tips(str_save_succeed);
		}else{
			save_tips(str_save_fail);
		}
	});
}
/* 保存提示 */
function save_tips(msg){
	var $save_tips=$("span.save_tips");
	if($save_tips.length==0){
		$("#save").before("<span class='save_tips'>"+msg+"</span>");
	}else{
		$("span.save_tips").html(msg).show();
	}
	if(msg != str_saving){
		$("span.save_tips").fadeOut(2000);
	}
	if(msg == str_saving||msg == str_save_succeed){
		$("span.save_tips").css("color","#000");
	}
}
function error_tips(msg,id){
	$(id).text(msg).css("color","#F00");
}

/* 测试提示 */
function test_tips(msg){
	var $test_tips=$("span.save_tips");
	if($test_tips.length==0){
		$("#save").before("<span class='save_tips'>"+msg+"</span>");
	}else{
		$("span.save_tips").html(msg).show();
	}
	if(msg != str_testing){
		$("span.save_tips").fadeOut(2000,function(){
			$("span.save_tips").remove();
		});
	}
}

/* 重启摄像机 */
function reboot(progressBarId,progressTime,httpport){
	showProgress(progressBarId,progressTime,httpport);
}
/* 显示进度条 */
function showProgress(progressBarId,progressTimeMs,httpport){
	//显示遮盖成
	$("#coverlayer",window.parent.document).show();
	//初始化进度条
	$(progressBarId).progressbar({
		value: false,
		change: function() {
			$(progressBarId+" .progress-label").text($(progressBarId).progressbar( "value" ) + "%" );
		},
		complete: function() {
			$(progressBarId+" .progress-label").text(str_complete);
			setTimeout(function(){
//				window.parent.location.href = "http://"+document.location.hostname+":"+httpport;
				top.location.href = "/";
			},2000);
		}
	});
	//显示进度条
	$(progressBarId).show();
	function progressChange(){
		var val = $(progressBarId).progressbar( "value" );
		$(progressBarId).progressbar( "value", val + 1 );
		if ( val < 99 ) {
			setTimeout(progressChange,progressTimeMs);
		}
	}
	setTimeout(progressChange,1000);
}
/* 注销 */
function clear_auth(){  
	try{  
	  if(GetBrowserVer().brower == "IE"){  
		  //IE浏览器实现注销功能
		document.execCommand("ClearAuthenticationCache");  
	  }else if(GetBrowserVer().brower == "Firefox"){
		  //Firefox实现注销功能
		/*var xmlhttp = createXMLObject();  
		xmlhttp.open("GET",".force_logout_offer_login_mozilla",true,"logout","logout");  
		xmlhttp.send("");  
		xmlhttp.abort();*/
		$.ajax({
			type:"GET",
			url:".force_logout_offer_login_mozilla",
			async:false
		});
	  }else{
		 //Google Chrome等浏览器实现注销功能
		/* var xmlHttp = false;  
		 if(window.XMLHttpRequest){
			xmlHttp = new XMLHttpRequest();  
		 }
		 xmlHttp.open("GET", "./", false, "logout", "logout");    
		 xmlHttp.send(null);  
		 xmlHttp.abort(); */
		 /*$.ajax({
			type: "GET",
			url: "./index.html",
			cache: false,
	 		username:"logout",
			password:"logout",
			dataType:"HTML"
		 });*/
		 $.ajax({
			type:"GET",
			url:"./",
			async:false
		});
		//$.get("./");
	  }
	}catch(e){
		return false;  
	}
	window.location.reload();
	//parent.window.location = parent.window.location;  
}

/* 时间格式化字符串函数 */
Date.prototype.Format = function(fmt){
	var o = {   
		"M+" : this.getMonth()+1,
		"d+" : this.getDate(),
		"h+" : this.getHours(),
		"m+" : this.getMinutes(),
		"s+" : this.getSeconds(),
		"q+" : Math.floor((this.getMonth()+3)/3),
		"S"  : this.getMilliseconds()
	};   
	if(/(y+)/.test(fmt))
		fmt = fmt.replace(RegExp.$1, (this.getFullYear()+"").substr(4 - RegExp.$1.length));   
	for(var k in o){
	if(new RegExp("("+ k +")").test(fmt))
		fmt = fmt.replace(RegExp.$1, (RegExp.$1.length==1) ? (o[k]) : (("00"+ o[k]).substr((""+ o[k]).length)));
	}// for
	return fmt;
}

/* base64 codec */
var Base64 = 
{
    _keyStr: "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",
    encode: function(b) {
        for (var d = "",
        c, a, f, g, h, e, i = 0,
        b = Base64._utf8_encode(b); i < b.length;) c = b.charCodeAt(i++),
        a = b.charCodeAt(i++),
        f = b.charCodeAt(i++),
        g = c >> 2,
        c = (c & 3) << 4 | a >> 4,
        h = (a & 15) << 2 | f >> 6,
        e = f & 63,
        isNaN(a) ? h = e = 64 : isNaN(f) && (e = 64),
        d = d + this._keyStr.charAt(g) + this._keyStr.charAt(c) + this._keyStr.charAt(h) + this._keyStr.charAt(e);
        return d
    },
    decode: function(b) {
        for (var d = "",
        c, a, f, g, h, e = 0,
        b = b.replace(/[^A-Za-z0-9\+\/\=]/g, ""); e < b.length;) c = this._keyStr.indexOf(b.charAt(e++)),
        a = this._keyStr.indexOf(b.charAt(e++)),
        g = this._keyStr.indexOf(b.charAt(e++)),
        h = this._keyStr.indexOf(b.charAt(e++)),
        c = c << 2 | a >> 4,
        a = (a & 15) << 4 | g >> 2,
        f = (g & 3) << 6 | h,
        d += String.fromCharCode(c),
        g != 64 && (d += String.fromCharCode(a)),
        h != 64 && (d += String.fromCharCode(f));
        return d = Base64._utf8_decode(d)
    },
    _utf8_encode: function(b) {
		var d = "";
        for (var c = 0; c < b.length; c++) {
            var a = b.charCodeAt(c);
            a < 128 ? d += String.fromCharCode(a) : (a > 127 && a < 2048 ? d += String.fromCharCode(a >> 6 | 192) : (d += String.fromCharCode(a >> 12 | 224), d += String.fromCharCode(a >> 6 & 63 | 128)), d += String.fromCharCode(a & 63 | 128))
        }
        return d
    },
    _utf8_decode: function(b) {
        for (var d = "",
        c = 0,
        a = c1 = c2 = 0; c < b.length;) a = b.charCodeAt(c),
        a < 128 ? (d += String.fromCharCode(a), c++) : a > 191 && a < 224 ? (c2 = b.charCodeAt(c + 1), d += String.fromCharCode((a & 31) << 6 | c2 & 63), c += 2) : (c2 = b.charCodeAt(c + 1), c3 = b.charCodeAt(c + 2), d += String.fromCharCode((a & 15) << 12 | (c2 & 63) << 6 | c3 & 63), c += 3);
        return d
    }
};

/* 页面导航处理 */

if(chkCookie("username"))
{
	// 用户已登录时访问登录界面时跳转到主页面
	if(document.URL.indexOf("index.html")>0){
		window.location.href="admin.html";
	}
}
else 
{

	// 用户未登录时跳转到登录界面
	if(document.URL.indexOf("index.html")<0)
	{
		document.write(document.URL)
		if(typeof(window.top)!="undefined")
		{
			window.top.location.href="index.html";
 		}
		else
		{
			window.location.href="index.html";
   		}
	}
}


/* 加载语言 */
var language_val=parseInt(getCookie("language"));
//根据浏览器初始化语言
if(isNaN(language_val)){
	var navlanguage = (GetBrowserVer().brower == "IE"?navigator.systemLanguage:navigator.language).toLowerCase();
	if(navlanguage == "zh-cn"){
		//简体中文
		language_val = 0;
	}else if(navlanguage == "en-us"){
		//英文
		language_val = 1;
	}else if(navlanguage == "ja"){
		//日语
		language_val = 2;
	}else if(navlanguage == "ko"){
		//韩语
		language_val = 3;
	}else if(navlanguage == "ru"){
		//俄语
		language_val = 2;
	}
}

switch(language_val){
	// 简体中文
	case 0:document.write('<script type="text/javascript" src="language/simple_chinese.js"></script>');break;
	// 英文
	case 1:document.write('<script type="text/javascript" src="language/english.js"></script>');break;		    
	//日语
	case 2:document.write('<script type="text/javascript" src="language/japanese.js"></script>');break;
	//韩语
	case 3:document.write('<script type="text/javascript" src="language/korea.js"></script>');break;
	/*//俄语
	case 2:document.write('<script type="text/javascript" src="language/russian.js"></script>');break;
	*/
	// 默认英文
	default:language_val=1;document.write('<script type="text/javascript" src="language/english.js"></script>')
}
/* 加载分辨率 */
var resolution_val = parseInt(getCookie("resolution")==null?11:getCookie("resolution"));

/* start camera funcion define */
var with_multi_cam = 0;
var with_ptz = 1;
var with_audio = 1;
var with_playback = 0;
var with_irctrl = 1;
/* end camera function start */

var camera_model=getCookie("camera_model");
var oem_info=getCookie("oem_info");

if(camera_model == "IP391"){
	with_ptz = 0;
	with_audio = 0;
	with_irctrl = 0;
}else if(camera_model == "MINI319"){
	with_ptz = 0;
}
