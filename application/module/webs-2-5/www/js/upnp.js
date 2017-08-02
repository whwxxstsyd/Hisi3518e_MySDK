var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){
	initParams();
	$("#save").bind("click",save);
});

function paramsImpl(){
	this.getParams=function(){
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?upnp_enb&p2p_enb&uid&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"upnp":upnp_enb,
					"p2p":p2p_enb
				};
			}
		});
		return res;
	}
	this.setParams=function(obj){
		var res=null;
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
		cgiurl+="&upnp_enb="+obj.upnp;
		cgiurl+="&p2p_enb="+obj.p2p;
		$.get(cgiurl);
	}
}

function initParams(){
	$("li.switch_icon").click(function(){
		if($(this).hasClass("switch_icon_on")){
			$(this).removeClass("switch_icon_on");
		}else{
			$(this).addClass("switch_icon_on");
		}
	});
	var paramObj=new paramsImpl();
	var param=paramObj.getParams();
	if(param.upnp=="on"){
		$("ul#upnp_switch li.switch_icon").addClass("switch_icon_on");
	}
	if(param.p2p=="on"){
		$("ul#p2p_switch li.switch_icon").addClass("switch_icon_on");
	}
}

function save(){
	var obj=new Object();
	obj.upnp=($("ul#upnp_switch li.switch_icon").hasClass("switch_icon_on")?"on":"off");
	obj.p2p=($("ul#p2p_switch li.switch_icon").hasClass("switch_icon_on")?"on":"off");
	new paramsImpl().setParams(obj);
	save_tips(str_save_succeed);	
}
