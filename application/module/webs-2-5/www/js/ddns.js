var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){
	loadDDNSParams();
	$("#save").bind("click",save);
});

function ddnsImpl(){
	this.getParams=function(){
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?CFG_ddns&http_port&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"innerEnable":inner_enb, 
					"innerUser":inner_user,
					"innerServer":inner_server,
					"httpPort":http_port,
					"thirdPartyEnable":thr_enb,
					"thirdPartyServer":thr_server,
					"thirdPartyUser":thr_user,
					"thirdPartyDomain":thr_domain,
					"innerState":inner_status,
					"thirdState":thr_status,					
				};
			}
		});
		return res;
	}
	this.setParams=function(obj){
		var res=null;
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
		cgiurl+="&inner_enb="+obj.innerEnable;
		cgiurl+="&thr_enb="+obj.thirdPartyEnable;
		cgiurl+="&thr_server="+obj.thirdPartyServer;
		cgiurl+="&thr_user="+obj.thirdPartyUser;
		cgiurl+="&thr_pwd="+obj.thirdPartyPwd;
		cgiurl+="&thr_domain="+obj.thirdPartyDomain;
		$.get(cgiurl);
	}
}

function loadDDNSParams(){
	//内置DDNS
	$("#innerDDNS li.switch_icon").click(function(){
		if($(this).hasClass("switch_icon_on")){
			$(this).removeClass("switch_icon_on");
			$("tr[data=innerDDNS_item]").hide();
		}else{
			$(this).addClass("switch_icon_on");
			$("tr[data=innerDDNS_item]").show();
		}
	});
	var ddnsObj=new ddnsImpl();
	var paramObj=ddnsObj.getParams();
	if(paramObj.innerEnable=="on"){
		$("#innerDDNS li.switch_icon").addClass("switch_icon_on");
	}else{
		$("tr[data=innerDDNS_item]").hide();
	}
	var ddnsurl="http://"+paramObj.innerUser+"."+paramObj.innerServer;
	$("#innerDDNSURL").text(ddnsurl).attr("href",ddnsurl);
	if (paramObj.innerState == "update_ok")
		$("#innerStatus").text(str_ddns_update_ok);
	else
		$("#innerStatus").text(str_ddns_update_fail);
	//第三方ddns
	$("#thirdPartyDDNS li.switch_icon").click(function(){
		if($(this).hasClass("switch_icon_on")){
			$(this).removeClass("switch_icon_on");
			$("tr[data=thirdPartyDDNS_item]").hide();
		}else{
			$(this).addClass("switch_icon_on");
			$("tr[data=thirdPartyDDNS_item]").show();
		}
	});
	if(paramObj.thirdPartyEnable=="on"){
		$("#thirdPartyDDNS li.switch_icon").addClass("switch_icon_on");
	}else{
		$("tr[data=thirdPartyDDNS_item]").hide();
	}
	$("#thirdPartyDDNSServer").val(paramObj.thirdPartyServer);
	$("#thirdPartyUser").val(paramObj.thirdPartyUser);
	$("#thirdPartyPwd").val(paramObj.thirdPartyPwd);
	$("#thirdPartyDomain").val(paramObj.thirdPartyDomain);
}

function save(){
	var obj=new Object();
	obj.innerEnable=($("#innerDDNS li.switch_icon").hasClass("switch_icon_on")?"on":"off");
	obj.thirdPartyEnable=($("#thirdPartyDDNS li.switch_icon").hasClass("switch_icon_on")?"on":"off");
	obj.thirdPartyServer=$("#thirdPartyDDNSServer").val();
	obj.thirdPartyUser=$("#thirdPartyUser").val();
	obj.thirdPartyPwd=$("#thirdPartyPwd").val();
	obj.thirdPartyDomain=$("#thirdPartyDomain").val();
	new ddnsImpl().setParams(obj);
	save_tips(str_save_succeed);	
}
