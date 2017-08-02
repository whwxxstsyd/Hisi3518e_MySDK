var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){
	loadPortInfo();
	$("#save").bind("click",save);
});

function portInfoImpl(){
	this.getPortInfo=function(){
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?CFG_misc&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"httpport":http_port,
					"rtspport":rtsp_port,
					"onvifport":onvif_port,
				};
			}
		});
		return res;
	}
	this.setPortInfo=function(obj){
		var res=null;
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
		cgiurl+="&http_port="+obj.httpport;
		cgiurl+="&rtsp_port="+obj.rtspport;
		cgiurl+="&onvif_port="+obj.onvifport;
		$.get(cgiurl);
	}
}

function loadPortInfo(){
	//onvif开关控制
	/*$("#onvifswitch li.switch_icon").click(function(){
		if($(this).hasClass("switch_icon_on")){
			$(this).removeClass("switch_icon_on");
			$("tr[data=onvif_item]").show();
		}else{
			$(this).addClass("switch_icon_on");
			$("tr[data=onvif_item]").hide();
		}
	});*/
	var portInfoObj=new portInfoImpl();
	var portInfo=portInfoObj.getPortInfo();
/*	if(portInfo.dhcpip=="on"){
		$("#dhcpswitch li.switch_icon").addClass("switch_icon_on");
		$("tr[data=dhcpipsub]").hide();
	}*/
	$("#httpport").val(portInfo.httpport);
	$("#rtspport").val(portInfo.rtspport);
	$("#onvifport").val(portInfo.onvifport);
}

function save(){
	var obj=new Object();
	obj.httpport=$("#httpport").val();
	obj.rtspport=$("#rtspport").val();
	obj.onvifport=$("#onvifport").val();
	new portInfoImpl().setPortInfo(obj);
	save_tips(str_save_succeed);	
}
