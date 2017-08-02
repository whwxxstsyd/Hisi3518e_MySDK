var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));
var camera_model=getCookie("camera_model");

$(function(){
	
	if (camera_model == "241BA")
	{
			var obj = document.getElementById("Resolution0Sel");		
					
			obj.add(new Option("720P","720P"));				
	}
	else if (camera_model == "241AA")
	{
			var obj = document.getElementById("Resolution0Sel");		
			
			obj.add(new Option("1080P","1080P"));								
			obj.add(new Option("720P","720P"));					
	}
	
	initParams();

	// === 提交保存数据 ===
	$("#save").bind("click",save);
});

function VIDEO_MODEL(){
	this.getParams=function(){
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?&CFG_video&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"videoBitrate0":video_bitrate0,
					"videoFps0":video_fps0,					
					"videoResolution0":video_res0,	
					"videoBitrate1":video_bitrate1,
					"videoFps1":video_fps1,										
					"videoResolution1":video_res1,														
				};
			}
		});
		return res;
	}
	
	this.setParams=function(obj){
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
		cgiurl += "&video_res0=" + $("#Resolution0Sel").val();	
		cgiurl += "&video_fps0=" + $("#Fps0Sel").val();			
		cgiurl += "&video_bitrate0=" + $("#Bitrate0Sel").val();	
		cgiurl += "&video_res1=" + $("#Resolution1Sel").val();	
		cgiurl += "&video_fps1=" + $("#Fps1Sel").val();					
		cgiurl += "&video_bitrate1=" + $("#Bitrate1Sel").val();			
		$.get(cgiurl);
	}
}

function initParams(){
	var recordObj=new VIDEO_MODEL();
	var params=recordObj.getParams();
	
	$("#Resolution0Sel").val(params.videoResolution0);
	$("#Fps0Sel").val(params.videoFps0);		
	$("#Bitrate0Sel").val(params.videoBitrate0);	
	$("#Resolution1Sel").val(params.videoResolution1);
	$("#Fps1Sel").val(params.videoFps1);			
	$("#Bitrate1Sel").val(params.videoBitrate1);		
}

function save(){
	new VIDEO_MODEL().setParams();
	save_tips(str_save_succeed);	
}