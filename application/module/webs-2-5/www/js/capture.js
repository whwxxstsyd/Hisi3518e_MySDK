var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){
	initParams();
	// 定时抓拍开关切换
	$("#timingCapture_switch li.switch_icon").click(function(){
		if($(this).hasClass("switch_icon_on")){
			$(".captureSettings").hide();
		}else{
			$(".captureSettings").show();
		}
	});
		// 开关控制
	$("li.switch_icon").click(function(){
		if($(this).hasClass("switch_icon_on")){
			$(this).removeClass("switch_icon_on");
		}else{
			$(this).addClass("switch_icon_on");
		}
	});

	// === 提交保存数据 ===
	$("#save").bind("click",save);
});

function CAPTURE_MODEL(){
	this.getParams=function(){
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?&CFG_snap&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"captureEnable":snap_enb,
					"captureInterval":snap_sd_interval,						
					"SdEnable":snap_sd_enb,
					"EmailEnable":snap_email_enb,
					"FtpEnable":snap_ftp_enb,					
				};
			}
		});
		return res;
	}
	
	this.setParams=function(obj){
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
		cgiurl += "&snap_sd_interval=" + $("#SnapInterval").val();
		if($("#timingCapture_switch li.switch_icon").hasClass("switch_icon_on")){
			cgiurl += "&snap_enb=" + "on";
		}else{
			cgiurl += "&snap_enb=" + "off";
		}				
		if($("#SdCapture_switch li.switch_icon").hasClass("switch_icon_on")){
			cgiurl += "&snap_sd_enb=" + "on";
		}else{
			cgiurl += "&snap_sd_enb=" + "off";
		}
		if($("#EmailCapture_switch li.switch_icon").hasClass("switch_icon_on")){
			cgiurl += "&snap_email_enb=" + "on";
		}else{
			cgiurl += "&snap_email_enb=" + "off";
		}	
		if($("#FtpCapture_switch li.switch_icon").hasClass("switch_icon_on")){
			cgiurl += "&snap_ftp_enb=" + "on";
		}else{
			cgiurl += "&snap_ftp_enb=" + "off";
		}		
		$.get(cgiurl);
	}
}

function initParams(){
	var recordObj=new CAPTURE_MODEL();
	var params=recordObj.getParams();
	
		// 定时抓拍开关初始化
	if(params.captureEnable == "on"){
		$("#timingCapture_switch li.switch_icon").addClass("switch_icon_on");
		$(".captureSettings").show();
	}else{
		$(".captureSettings").hide();
	}
	if(params.SdEnable=="on"){
		$("ul#SdCapture_switch li.switch_icon").addClass("switch_icon_on");
	}
	if(params.EmailEnable=="on"){
		$("ul#EmailCapture_switch li.switch_icon").addClass("switch_icon_on");
	}
	if(params.FtpEnable=="on"){
		$("ul#FtpCapture_switch li.switch_icon").addClass("switch_icon_on");
	}	
	$("#SnapInterval").val(params.captureInterval);		
}

function save(){
	new CAPTURE_MODEL().setParams();
	save_tips(str_save_succeed);		
}