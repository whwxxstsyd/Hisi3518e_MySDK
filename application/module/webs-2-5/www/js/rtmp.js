var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){
	loadParams();
	$("#save").bind("click",save);
});

function RTMP_MODEL(){
	this.getParams=function(){
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?CFG_rtmp&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"rtmpPort":rtmp_port,
					"rtmpPushEnb":rtmp_push_enb,
					"rtmpPushUrl":rtmp_push_url,
				};
			}
		});
		return res;
	}
	this.setParams=function(obj){
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
		
		cgiurl+="&rtmp_port="+$("#rtmpport").val();
		if($("ul#rtmppush_switch li.switch_icon").hasClass("switch_icon_on"))
			cgiurl += "&rtmp_push_enb=" + "on";
		else
			cgiurl += "&rtmp_push_enb=" + "off";		
		cgiurl+="&rtmp_push_url="+$("#rtmpurl").val();
		
		$.get(cgiurl);
	}
}


function loadParams(){
	// 开关控制
	$("li.switch_icon").click(function(){
		if($(this).hasClass("switch_icon_on")){
			$(this).removeClass("switch_icon_on");
		}else{
			$(this).addClass("switch_icon_on");
		}
	});

	var rtmpObj=new RTMP_MODEL().getParams();
	
	$("#rtmpport").val(rtmpObj.rtmpPort);
	$("#rtmpurl").val(rtmpObj.rtmpPushUrl);
	if(rtmpObj.rtmpPushEnb=="on"){
		$("ul#rtmppush_switch li.switch_icon").addClass("switch_icon_on");
	}
}

function save(){
	new RTMP_MODEL().setParams();
	save_tips(str_save_succeed);	
}
