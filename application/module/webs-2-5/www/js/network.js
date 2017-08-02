var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){
	loadNetInfo();
	$("#save").bind("click",save);
});

function netInfoImpl(){
	this.getNetInfo=function(){
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?CFG_net&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"dhcpip":dhcp,
					"ip":static_ip,//ip地址
					"subnetmask":static_netmask,//子网掩码
					"gateway":static_gateway,//网关地址
					"preferredDNS":static_dns1,//首先dns
					"alternateDNS":static_dns2//备用dns
				};
			}
		});
		return res;
	}
	
	this.setNetInfo=function(obj){
		var res=null;
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
		cgiurl+="&dhcp="+obj.dhcp;
		cgiurl+="&static_ip="+obj.ip;
		cgiurl+="&static_netmask="+obj.subnetmask;
		cgiurl+="&static_gateway="+obj.gateway;
		cgiurl+="&static_dns1="+obj.preferredDNS;
		cgiurl+="&static_dns2="+obj.alternateDNS;
		$.get(cgiurl);
	}
}

function loadNetInfo(){
	// dhcp开关控制
	$("#dhcpswitch li.switch_icon").click(function(){
		if($(this).hasClass("switch_icon_on")){
			$(this).removeClass("switch_icon_on");
			$('tr[data="dhcpipsub"]').show();
			if($("#autodnsipswitch li.switch_icon").hasClass("switch_icon_on")){
				$("#autodnsipswitch li.switch_icon").click();
			}
		}else{
			$(this).addClass("switch_icon_on");
			$('tr[data="dhcpipsub"]').hide();
			if(!$("#autodnsipswitch li.switch_icon").hasClass("switch_icon_on")){
				$("#autodnsipswitch li.switch_icon").click();
			}
		}
	});
	// 自动获取DNS服务器地址开关控制
	$("#autodnsipswitch li.switch_icon").click(function(){
		if($(this).hasClass("switch_icon_on")){
			$(this).removeClass("switch_icon_on");
			$('tr[data="dhcpdnssub"]').show();
		}else{
			if(!$("#dhcpswitch li.switch_icon").hasClass("switch_icon_on")){
				return false;
			}
			$(this).addClass("switch_icon_on");
			$('tr[data="dhcpdnssub"]').hide();
		}
	});
	var netInfoObj=new netInfoImpl();
	var netInfo=netInfoObj.getNetInfo();
	if(netInfo.dhcpip=="dhcp_all"){
		$("#dhcpswitch li.switch_icon").addClass("switch_icon_on");
		$("#autodnsipswitch li.switch_icon").addClass("switch_icon_on");		
		$("tr[data=dhcpipsub]").hide();
		$("tr[data=dhcpdnssub]").hide();
	}
	if(netInfo.dhcpip=="dhcp_ip"){
		$("#dhcpswitch li.switch_icon").addClass("switch_icon_on");
		$("tr[data=dhcpipsub]").hide();
	}
	$("#ipaddr").val(netInfo.ip);
	$("#mask").val(netInfo.subnetmask);
	$("#gateway").val(netInfo.gateway);
	$("#prefdns").val(netInfo.preferredDNS);
	$("#altedns").val(netInfo.alternateDNS);
}

function save(){
	var obj=new Object();
	if ($("#dhcpswitch li.switch_icon").hasClass("switch_icon_on") && $("#autodnsipswitch li.switch_icon").hasClass("switch_icon_on"))
			obj.dhcp = "dhcp_all";
	else if ($("#dhcpswitch li.switch_icon").hasClass("switch_icon_on"))
			obj.dhcp = "dhcp_ip";
	else
			obj.dhcp = "dhcp_off";
	obj.ip=$("#ipaddr").val();
	obj.subnetmask=$("#mask").val();
	obj.gateway=$("#gateway").val();
	obj.preferredDNS=$("#prefdns").val();
	obj.alternateDNS=$("#altedns").val();
	new netInfoImpl().setNetInfo(obj);
	save_tips(str_save_succeed);	
}
