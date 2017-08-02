var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){	
	//开关控制
	$("li.switch_icon").click(function(){
		if($(this).hasClass("switch_icon_on")){
			$(this).removeClass("switch_icon_on");
		}else{
			$(this).addClass("switch_icon_on");
		}
	});
	//灵敏度进度条
	$(".slider").slider({
		range: "min",
		slide:function(evt,ui){
			$(this).next("li").html(ui.value);
	    }
	});
	loadParams()
	
	$("#save").bind("click",save);
});

function SOUND_MODEL(){
	this.getParams=function(){
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?CFG_audio&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"SpeakerVol":out_vol, 
					"MicVol":in_vol, 					
					"MicEnable":mic_enable,
				};
			}
		});
		return res;
	}
	this.setParams=function(){
		var res=null;
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
	
		cgiurl += "&out_vol=" + $("#output_vol").slider("value");
		cgiurl += "&in_vol=" + $("#input_vol").slider("value");		
/*		
		if($("#micEnable_switch" + " li.switch_icon").hasClass("switch_icon_on")){
			cgiurl += "&mic_enable" + "=on";
		}else{
			cgiurl += "&mic_enable" + "=off";
		}		
*/		
		$.get(cgiurl);
	}
}

function loadParams(){		
	/* 输出音量 */
	var mdtObj=new SOUND_MODEL();
	var pamams=mdtObj.getParams();
	
//	if(pamams.MicEnable=="on")
//			$("#micEnable_switch" + " li.switch_icon").addClass("switch_icon_on");
			
			
	$("#input_vol").slider("value", pamams.MicVol);
	$("#input_vol").next("li").text(pamams.MicVol);
				
	$("#output_vol").slider("value", pamams.SpeakerVol);
	$("#output_vol").next("li").text(pamams.SpeakerVol);
}

function save(){
	new SOUND_MODEL().setParams();
	save_tips(str_save_succeed);	
}
