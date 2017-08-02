var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

var Player = new PlayerObj();
var params=null;
var VideoBrightnessInit = 0;
var VideoContastInit = 0;
var ListenFlag = 0;
var TalkFlag = 0;
function initParams(){
	var cgiurl="/cgi-bin/get_param.cgi?user="+uname+"&pwd="+upwd+"&pwr_freq&web_resolution&ir_led&CFG_video";
	$.ajax({
		url:cgiurl,
		async:false,
		cache:false,
		success: function(data){
			eval(data);	
			params={
				"powerfre":pwr_freq,//电源频率
				"resolution":web_resolution,//分辨率
				"ir_led":ir_led,//红外灯
				"brightness":video_brightness,//亮度
				"contrast":video_contrast,//对比度
				"flip":video_flip//视频翻转
			};
		}
	});
}

$(function(){
	// 下拉菜单
	$(".menuPanel dt").click(function(){
		$(this).parent("dl").siblings("dl").children("dd").slideUp(100);
		var $dd=$(this).siblings("dd");
		if($dd.css("display")=="none"){
			$dd.slideDown(100);
			if($(this).parents("#ctrlmenu") != "undefined"){
				$(this).next("dd").children("a").click();
			}
		}else{
			$dd.slideUp(100);
		}
	});
	
	// 下拉菜单初始样式
	$(".menuPanel dl").siblings("dl").children("dd").slideUp(100);
//	$(".menuPanel dl").first().children("dd").eq(0).children("a").css("font-weight","bold");	
	
//	Player.Load("#PlayBox");	
//	videoPlay(); 		//视频播放		
	initParams();		/* 加载参数 */	
	resolution(); 	//分辨率
	videoAdj(); 		//视频调节
	videoFlip(); 		//图像翻转	
	powerFreq(); 		//电源频率	
	capture(); 			//抓拍
	record(); 			//录像
	ptzCtrl(); 			//云台控制
	zoom(); 				//变焦
	preset(); 			//预置位
	irLed(); 				//红外灯
	talk();					//对讲
	listen();				//监听

	
	if (with_ptz == 1){
		$("#ptz").show();
		$("#patrol").show();
		$("#preset").show();	
	}	
	if (with_irctrl == 1)
		$("#infrdLight").show();
	if (with_multi_cam == 1)
		$('.multiDev').show();
/* 用户权限管理 */		
	if (uname == "opt")
	{
		var ui;
		ui =document.getElementById("selectPanel");
		ui.style.visibility="hidden";
		ui =document.getElementById("ctrlmenu");
		ui.style.visibility="hidden";
	}
	else if (uname == "guest")
	{
		$("#ctrPanel").hide();
	}		
	
	$("#talk_btn").attr("title",str_talk);
	$("#listen_btn").attr("title",str_title_monitor);	
});

/******************  处理界面显示  *********************/
function videoPlay()			/* 视频播放 */
{
		Player.Play("#videoPlay");			
}

function fullSCreen()			/* 全屏 */
{
	
}

function talk()
{
	$("#talk_btn").click(function(){
		if (TalkFlag == 0)
			TalkFlag = 1;
		else
			TalkFlag = 0;

		Player.talk(TalkFlag);		
	});	
}

function listen()
{
	$("#listen_btn").click(function(){
		if (ListenFlag == 0)
			ListenFlag = 1;
		else
			ListenFlag = 0;
		Player.listen(ListenFlag);
	});	
}

function capture()				/* 抓拍 */
{
//	$("#ptzMiddle").attr("title",str_title_snap);
	$("#ptzMiddle").click(function(){
		 	window.open("/cgi-bin/snapshot.cgi?resolution=720P&user=" + uname + "&pwd=" + upwd);
	});
}

function record()					/* 录像 */
{
	
}

function hello()
{
		alert("hello");
	}

function ptzCtrl()		/* 云台控制 */
{
	//	$("#ptzUpLeft").bind("mousedown",ptzCtrlImpl("upLeft"));			有问题
	$("#ptzUpLeft").mousedown(function(){ptzCtrlImpl("up&ptz_ctrl=left");});	
	$("#ptzUp").mousedown(function(){ptzCtrlImpl("up")});
	$("#ptzUpRight").mousedown(function(){ptzCtrlImpl("up&ptz_ctrl=right")});	
	$("#ptzLeft").mousedown(function(){ptzCtrlImpl("left")});	
	$("#ptzRight").mousedown(function(){ptzCtrlImpl("right")});	
	$("#ptzDownLeft").mousedown(function(){ptzCtrlImpl("down&ptz_ctrl=left")});
	$("#ptzDown").mousedown(function(){ptzCtrlImpl("down")});	
	$("#ptzDownRight").mousedown(function(){ptzCtrlImpl("down&ptz_ctrl=right")});		
	
	$("#ptzUpLeft").mouseup(function(){ptzCtrlImpl("stop")});	
	$("#ptzUp").mouseup(function(){ptzCtrlImpl("stop")});
	$("#ptzUpRight").mouseup(function(){ptzCtrlImpl("stop")});	
	$("#ptzLeft").mouseup(function(){ptzCtrlImpl("stop")});		
	$("#ptzRight").mouseup(function(){ptzCtrlImpl("stop")});	
	$("#ptzDownLeft").mouseup(function(){ptzCtrlImpl("stop")});
	$("#ptzDown").mouseup(function(){ptzCtrlImpl("stop")});	
	$("#ptzDownRight").mouseup(function(){ptzCtrlImpl("stop")});			
	
	$("#patrol_h").click(function(){ptzCtrlImpl("updown")});
	$("#patrol_stop").click(function(){ptzCtrlImpl("stop")});
	$("#patrol_v").click(function(){ptzCtrlImpl("leftright")});	
}

function zoom()		/* 变焦 */
{

}
function preset()		/* 预置位 */
{
	var presetObj=new presetImpl();
	var html="";			/* 加载预置位 */
	
	for(var i=1;i<9;i++)
			html+='<span class="preval">'+i+'</span>';
	$("dl#preset dd").prepend(html);
	
	/* 选择预置位 */
	$("span.preval").click(function()	{
		if($(this).hasClass("currpreval")){
			$(this).removeClass("currpreval");
		}else{
			$(this).siblings("span.currpreval").removeClass("currpreval");
			$(this).addClass("currpreval");
		}
	});
	/* 设置预置位 */
	$("span#set_pre").click(function(){
		if($("span.currpreval").text()==""){
			return;
		}
		$("span.currpreval").css("color","#000");
		presetObj.setPreset(parseInt($("span.currpreval").text())-1);		
	});
	/* 调用预置位 */
	$("span#go_pre").click(function(){
		if($("span.currpreval").text()==""){
			return;
		}
		presetObj.gotoPreset(parseInt($("span.currpreval").text())-1);
	});
	/* 清除预置位 */
	$("span#clear_pre").click(function(){
		if($("span.currpreval").text()==""){
			return;
		}
		$("span.currpreval").css("color","#FFF");
		$("span.currpreval").removeClass("currpreval");
		presetObj.clearPreset(parseInt($("span.currpreval").text())-1);		
	});
}

/* 设置电源频率 */
function powerFreq(){
	var pfre=new powerFreqImpl();
	$("#powerfre").val(pfre.getPowerFreq());
	$("#powerfre").change(function(){
		pfre.setPowerFreq(this.value);
	});
}

/* 设置分辨率 */
function resolution(){
	var res=new resolutionImpl();
	$("#resolution").val(res.getRes());
	$("#resolution").change(function(){
		res.setRes(this.value);
	});
}

/* 红外灯控制 */
function irLed(){
	var irLedObj=new irLedImpl();
	$("#irled").val(irLedObj.getIrLed());
	$("#irled").change(function(){
		irLedObj.setIrLed(this.value);
	});
}

/* 视频效果调节 */
function videoAdj()
{
	var adjObj=new videoAdjImpl();
	var adjParams=adjObj.getAdjParams();
	
	$(".slider").slider({
		orientation: "horizontal",
		range: "min",
		max: 100,
		slide:function(evt,ui){
			$(this).next("span").text(ui.value);
		},
		change:function(evt,ui){
			$(this).next("span").text(ui.value);
			var $prev=$(this).prev("span");
			
			if($prev.attr("id") == "brightness"){
				if (VideoBrightnessInit == 0)
					VideoBrightnessInit = 1;
				else
			 		adjObj.setBrightness(ui.value);
			}else if($prev.attr("id") == "contrast"){
				if (VideoContastInit == 0)
					VideoContastInit = 1;
				else
			 		adjObj.setContrast(ui.value);
			}
		}
	});	
	
		/* 亮度 */
	$("span#brightness").attr("title",str_brightness);
	$("span#brightness").next("div.slider").slider("value",adjParams.brightness);
	$("span#brightness").nextAll("span").text(adjParams.brightness);
	/* 对比度 */
	$("span#contrast").attr("title",str_contrast);
	$("span#contrast").next("div.slider").slider("value",adjParams.contrast);
	$("span#contrast").nextAll("span").text(adjParams.contrast);		
	
	$("#videoParamDefault").click(function(){
		$(".slider").slider("value",50);	
	});	
}

function videoFlip()
{
	var flipObj=new videoFlipImpl();
	var filpVal=flipObj.getFilp();
	
	if(filpVal.invert=="on")
			$("ul#invert li.switch_icon").addClass("switch_icon_on");
	if(filpVal.mirror=="on")
			$("ul#mirror li.switch_icon").addClass("switch_icon_on");
			
	$("ul#invert li.switch_icon,ul#mirror li.switch_icon").click(function(){
		var flip = {
			"invert":"",
			"mirror":""
		};
		
		if($(this).hasClass("switch_icon_on"))
			$(this).removeClass("switch_icon_on");
		else
			$(this).addClass("switch_icon_on");
		
		if($("ul#invert li.switch_icon").hasClass("switch_icon_on"))
			flip.invert="on";
		else
			flip.invert="off";
		
		if($("ul#mirror li.switch_icon").hasClass("switch_icon_on"))
			flip.mirror="on";
		else
			flip.mirror="off";
			
		flipObj.setFilp(flip);
	});
}

/*******************  与后台交互获取数据  *******************/
/* 视频播放 */
function videoPlayImpl(){
}

/* 全屏 */
function fullSCreenImpl(){
	
}

/* 抓拍 */
function captureImpl(){
}

/* 录像 */
function recordImpl(){
	
}

/* 云台控制 */
function ptzCtrlImpl(action){
	var cgiurl="/cgi-bin/cmd_proc.cgi?user="+uname+"&pwd="+upwd+"&ptz_ctrl="+action	+"&_="+new Date().getTime();
	$.get(cgiurl);
}

/* 预置位 */
function presetImpl(){
	this.setPreset=function(val){
		var cgiurl="/cgi-bin/cmd_proc.cgi?user="+uname+"&pwd="+upwd+"&set_pos="+val	+"&_="+new Date().getTime();
		$.get(cgiurl);
	}
	this.gotoPreset=function(val){
		var cgiurl="/cgi-bin/cmd_proc.cgi?user="+uname+"&pwd="+upwd+"&goto_pos="+val +"&_="+new Date().getTime();
		$.get(cgiurl);
	}
	this.clearPreset=function(val){
		var cgiurl="/cgi-bin/cmd_proc.cgi?user="+uname+"&pwd="+upwd+"&clr_pos="+val	+"&_="+new Date().getTime();
		$.get(cgiurl);
	}		
}

/* 电源频率 */
function powerFreqImpl(){
	this.getPowerFreq=function(){
		return params.powerfre;
	}
	this.setPowerFreq=function(val){
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd+"&pwr_freq="+val	+"&_="+new Date().getTime();
		$.get(cgiurl);
	}
}

/* 分辨率 */
function resolutionImpl(){
	this.getRes=function(){
		return params.resolution;
	}
	this.setRes=function(val){
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd+"&web_resolution="+val	+"&_="+new Date().getTime();
		$.get(cgiurl);
	}
}

/* 红外灯控制 */
function irLedImpl(){
	this.getIrLed=function(){
		var rs=params.ir_led;
		return rs;
	}
	this.setIrLed=function(val){
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd+"&ir_led="+val	+"&_="+new Date().getTime();
		$.get(cgiurl);
	}
}

/* 图像参数调节 */
function videoAdjImpl(){
	this.getAdjParams=function(){
		var rs={
			"brightness":params.brightness,
			"contrast":params.contrast
		};
		return rs;
	}
	this.setBrightness=function(val){
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd+"&video_brightness="+val	+"&_="+new Date().getTime();
		$.get(cgiurl);
	}
	this.setContrast=function(val){
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd+"&video_contrast="+val	+"&_="+new Date().getTime();
		$.get(cgiurl);
	}
}

/* 图像翻转 */
function videoFlipImpl(){
	this.getFilp=function(){
		var rs={
			"invert":"",//水平
			"mirror":""//垂直
		};
		if(params.flip=="flip_normal"){
			//水平、垂直都不翻转
			rs.invert="off";
			rs.mirror="off";
		}else if(params.flip=="flip_v"){
			//仅垂直翻转
			rs.invert="off";
			rs.mirror="on";
		}else if(params.flip=="flip_h"){
			//仅水平都翻转
			rs.invert="on";
			rs.mirror="off";
		}else if(params.flip=="flip_hv"){
			//水平垂直都翻转
			rs.invert="on";
			rs.mirror="on";
		}
		return rs;
	}
	this.setFilp=function(filpobj){
		var val="";
		if(filpobj.invert=="off"&&filpobj.mirror=="off"){
			val="flip_normal";
		}else if(filpobj.invert=="off"&&filpobj.mirror=="on"){
			val="flip_v";
		}else if(filpobj.invert=="on"&&filpobj.mirror=="off"){
			val="flip_h";
		}else if(filpobj.invert=="on"&&filpobj.mirror=="on"){
			val="flip_hv";
		}
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd+"&video_flip="+val	+"&_="+new Date().getTime();
		$.get(cgiurl);
	}
}