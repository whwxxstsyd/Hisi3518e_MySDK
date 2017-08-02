var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){
	initParams();

	// === 提交保存数据 ===
	$("#save").bind("click",save);
});

function PTZ_MODEL(){
	this.getParams=function(){
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?&CFG_ptz&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"ptzSpeed":ptz_speed,				
					"ptzCircleCnt":ptz_circle_cnt,
					"ptzStartPos":ptz_start_pos,									
				};
			}
		});
		return res;
	}
	
	this.setParams=function(obj){
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
		cgiurl += "&ptz_speed=" + $("#SpeedSel").val();	
		cgiurl += "&ptz_circle_cnt=" + $("#CircleCnt").val();				
		cgiurl += "&ptz_start_pos=" + $("#StartPosSel").val();		
		$.get(cgiurl);
	}
}

function initParams(){
	var recordObj=new PTZ_MODEL();
	var params=recordObj.getParams();
	
	$("#SpeedSel").val(params.ptzSpeed);
	$("#CircleCnt").val(params.ptzCircleCnt);		
	$("#StartPosSel").val(params.ptzStartPos);	
}

function save(){
	new PTZ_MODEL().setParams();
	save_tips(str_save_succeed);	
}