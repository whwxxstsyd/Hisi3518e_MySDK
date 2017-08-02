var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){
	timeSetting();//时间设置项
	manualSetting();//手动设置
	pcSyncSetting();//与电脑同步设置
	ntpSyncSetting();//与服务器同步设置
	timeZone();//时间设置区
	$("#save").bind("click", save)//保存设置
});

function getInitParams(){
	var res=null;
	var cgiurl="/cgi-bin/get_param.cgi?CFG_time&user="+uname+"&pwd="+upwd;
	$.ajax({
		url:cgiurl,
		async:false,
		cache:false,
		success: function(data){
			eval(data);
			res={
				"sync_type":sync_type,				//同步类型
				"timenow":timenow,						//当前设备时间
				"ntpserver":ntp_server,				//时间同步服务器
				"interval":sync_interval,			//同步间隔
				"timezone":timezone,
				"dst":dst_enb
			};
		}
	});
	return res;
}

function save_value(dataobj){
	var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
	cgiurl+="&sync_type="+dataobj.ntpType;//同步类型
	if(dataobj.ntpType=="ntp_sync"){
		//与时间服务器同步
		cgiurl+="&ntp_server="+dataobj.ntpserver;
		cgiurl+="&sync_interval="+dataobj.ntpinterval;
	}else{
		cgiurl+="&set_time="+dataobj.time;
	}
	cgiurl+="&timezone="+dataobj.timezone;
	cgiurl+="&dst_enb="+dataobj.dst;
	$.get(cgiurl);
}

var params=null;
function timeSetting(){
	//设置方式
	$("#ntpType").change(function(){
		var val = this.value;
		if(val == "nomal_sync"){
			$("tr#manualsetTime").show();
			$("tr#syncPCTime,tr.syncTimeServe").hide();
		}else if(val == "pc_sync"){
			$("tr#syncPCTime").show();
			$("tr#manualsetTime,tr.syncTimeServe").hide();
		}else if(val == "ntp_sync"){
			$("tr.syncTimeServe").show();
			$("tr#manualsetTime,tr#syncPCTime").hide();
		}
	});
	params=getInitParams();
	$("#ntpType").val(params.sync_type).change();//绑定当前设置方式的值
	var y=params.timenow.substr(0,4);
	var m=params.timenow.substr(5,2);
	var d=params.timenow.substr(8,2);
	var h=params.timenow.substr(11,2);
	var s=params.timenow.substr(14,2);
	var ms=params.timenow.substr(17,2);
	var time=new Date(y,m,d,h,s,ms);
	
	$("#timenow").text(time.Format("yyyy-MM-dd hh:mm:ss"));
	setInterval(function(){
		time.setTime(time.getTime()+1000);
		$("#timenow").text(time.Format("yyyy-MM-dd hh:mm:ss"));//显示当前设备时间
	},1000);
}

function manualSetting(){
	//打印小时
	for(var i=0;i<=23;i++){
		$("#manualHour").append('<option value="'+(i<=9?"0"+i:i)+'">'+(i<=9?"0"+i:i)+'</option>');
	}
	//打印分钟及秒钟
	for(var i=0;i<=59;i++){
		$("#manualMinute").append('<option value="'+(i<=9?"0"+i:i)+'">'+(i<=9?"0"+i:i)+'</option>');
		$("#manualSecond").append('<option value="'+(i<=9?"0"+i:i)+'">'+(i<=9?"0"+i:i)+'</option>');
    }
	$("#datepicker").datepicker({dateFormat : 'yy-mm-dd'});
	
	var time=new Date().Format("yyyy-MM-dd hh:mm:ss");
	$("#datepicker").val(time.substr(0,4)+"-"+time.substr(5,2)+"-"+time.substr(8,2));
	$("#manualHour").val(time.substr(11,2));
	$("#manualMinute").val(time.substr(14,2));
	$("#manualSecond").val(time.substr(17,2));
}

 /* 与电脑同步 */
function pcSyncSetting(){
	//电脑同步子项参数
	setInterval(function(){
		$("#pctime").val(new Date().Format("yyyy-MM-dd hh:mm:ss"));
	},1000);
}

/* ntp同步 */
function ntpSyncSetting(){
	for(var i=1;i<=24;i++){
		$("#ntpinterval").append('<option value="'+i+'">'+i+'</option>');
	}
	$("#ntpserver").val(params.ntpserver);
	$("#ntpinterval").val(params.interval);
}

/* 时区设置 */
function timeZone(){
	$("#TimeZone").val(params.timezone);
	if(params.dst=="on"){
		$("ul#dstsw li.switch_icon").addClass("switch_icon_on");	
	}
	$("ul#dstsw li.switch_icon").click(function(){
		if($(this).hasClass("switch_icon_on")){
			$(this).removeClass("switch_icon_on");
		}else{
			$(this).addClass("switch_icon_on");
		}
	});
}

function save(){
	var ntpTypeVal=$("#ntpType").val();
	var obj=new Object();
	obj.ntpType=ntpTypeVal;//设置方式
	if(ntpTypeVal=="nomal_sync"){
		//手动设置
		obj.time=$("#datepicker").val()+" "+$("#manualHour").val()+":"+$("#manualMinute").val()+":"+$("#manualSecond").val();
	}else if(ntpTypeVal=="pc_sync"){
		//与电脑同步
		obj.time=$("#pctime").val();
	}else if(ntpTypeVal=="ntp_sync"){
		//与服务器同步
		obj.ntpserver=$("#ntpserver").val();
		obj.ntpinterval=$("#ntpinterval").val();
	}
	obj.timezone=$("#TimeZone").val();//时区
	obj.dst=$("ul#dstsw li.switch_icon").hasClass("switch_icon_on")?"on":"off";//夏令时
	save_value(obj);
	save_tips(str_save_succeed);	
}