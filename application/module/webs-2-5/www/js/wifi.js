var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){
	var loadWifiAttr;
	
	loadWifiAttr = loadWifiList();
	var ajaxData = loadWifiAttr.getAjaxData();
	if(1){
		$("#wifi_switch li.switch_icon").addClass("switch_icon_on");
	}else{
		$("#wifi_list").hide();
	}
	$("#dialog").dialog({
		title:str_setwifi,
		modal:true,
		autoOpen:false,
		draggable:false,
		resizable:false,
		width:320,
		buttons:[{
			text:str_btn_ok,
			click:function(){
				$("#connectwifi_msg").html(str_tips_connecting);				
				//var ajaxData = loadWifiAttr.getAjaxData();
				eval(ajaxData);
				/*  变量索引 */
				var $index = $("#dialog").attr("index");
				/* 数据验证 */
				var $key = $("#wifikey");
				if($key.val()==""){
					$("#connectwifi_msg").html('<span style="color:#f00;">'+str_error_wifikeyIsNull+'</span>');
					$key.focus();
					return;
				}
				if(!checkKey($("#wifi_secret").text(),$key.val().length)){
					$("#connectwifi_msg").html('<span style="color:#f00;">'+str_error_wifikeyLength+'</span>');
					$key.focus();
					return;
				}
				var cgiurl = "/cgi-bin/set_param.cgi?wifi_ssid="+encodeURIComponent($("#wifi_ssid").text());
				cgiurl +=	"&type=" + $("#wifi_type").text() + "&wifi_key=" + encodeURIComponent($key.val());
				cgiurl += "&wifi_auth=" + $("#wifi_auth").text() + "&wifi_enc=" + $("#wifi_enc").text();
				cgiurl += "&user="+uname+"&pwd="+upwd;
				$.get(cgiurl,function(data){
						var retry = 30;
						setTimeout(function check_status(){		
							var cgiurl = "/cgi-bin/cmd_proc.cgi?wifi_link" + "&user=" + uname + "&pwd=" + upwd;
							$.get(cgiurl, function(data)
							{
								eval(data);	
								if (wifi_status == "linked" && link_ssid == $("#wifi_ssid").text())
								{
									$("#connectwifi_msg").html(str_connected);
									setTimeout(function (){
										$("#dialog").dialog('close',location.reload());
									},1500);
								}
								else
								{
									retry = retry - 1;
									if (retry == 0)
										$("#connectwifi_msg").html(str_error_connectfail);	
									else
									{
										setTimeout(check_status, 1000);
									}
								}
							});
						},1000);
					/*
					if(wifi_status == "linked"){
						$("#connectwifi_msg").html(str_connected);
						setTimeout(function(){
							$("#dialog").dialog('close',location.reload());
						},2000);
					}else{
						$("#connectwifi_msg").html(str_error_connectfail);
					}
					*/
				});
			}
		},
		{
			text:str_btn_cancel,
			click:function(){
				$(this).dialog('close');
			}
		}]
	});
	/* 保存设置 */
	$("#save").click(function(){
		var cgiurl = "/cgi-bin/set_param.cgi?";
		if($("#wifi_switch li.switch_icon").hasClass("switch_icon_on"))
		{
			cgiurl += "wifi_enable=on";
		}else{
			cgiurl += "wifi_enable=off";
		}
		$.get(cgiurl);
		save_tips(str_save_succeed);	
	});
	/* wifi状态开关 */
	$("#wifi_switch li.switch_icon").click(function(){
		if($(this).hasClass("switch_icon_on")){
			$(this).removeClass("switch_icon_on");
			$("#wifi_list").hide();
		}else{
			$(this).addClass("switch_icon_on");
			$("#wifi_list").show();
			loadWifiAttr = loadWifiList();
		}
	});
	/* key输入限制 */
	$("#wifikey").keyup(function(){
		this.value=this.value.replace(/[^\w]/ig,'');
	});
	/* 隐藏key */
	$("#showkey").change(function(){
		var keyval = $("#wifikey").val();
		if(this.checked){
			$("#wifikey_wrap").html('<input id="wifikey" type="text" value="'+keyval+'"/>');
		}else{
			$("#wifikey_wrap").html('<input id="wifikey" type="password" value="'+keyval+'"/>');
		}
	});
});
/* 加载搜索到的无线网 */
function loadWifiList(){
	$("#wifiList").html('<ul class="loading"><li class="loading_icon">&nbsp;</li><li>'+str_loading+'</li></ul>');
	var cgiurl = "/cgi-bin/cmd_proc.cgi?get_ap&wifi_link&user="+uname+"&pwd="+upwd +"&time="+new Date().getTime();
	var ajaxData;
	$.get(cgiurl,function(data){
		eval(data);
		ajaxData = data;
		// 信号强度
		var signal_arr = new Array();
		// 信号强度排序后对应的索引
		var sort_arr = new Array();
		var index_arr = new Array();
		for(var i=0;i<waccess_points;i++){
			signal_arr[i] = sort_arr[i] = wrssi[i];
		}
		// 排序
		function sortNumber(a,b){
			return b-a;
		}
		sort_arr.sort(sortNumber);
		for(var i=0;i<waccess_points;i++){
			for(var j=0;j<waccess_points;j++){
				if(sort_arr[i] == signal_arr[j]){
					signal_arr[j] = -1;
					index_arr[i] = j;
					break;
				}
			}
		}
		var html = '<div style="*zoom:1;"><table width="100%" cellspacing="0" style="overflow:scroll">';
//		var html = '<div style="*zoom:1;"><table width="100%" border="1" style="overflow:scroll">';
		for(var j=0;j<waccess_points;j++){
			try{
				var i = index_arr[j];
				var secret;
					if(wauth[i].indexOf("WPA2")!=-1){
						secret="WPA2-PSK";
					}else if(wauth[i].indexOf("WPA")!=-1){
						secret="WPA-PSK";
					}else{
						secret="WEP";
					}
				
				html += '<tr index="'+i+'" >';
				html += '<td><div class="ssid" title="'+wessid[i]+'">'+wessid[i]+'</div></td>';
				if(wessid[i] == link_ssid){
//					html += '<td class="connect_state" align="right">'+str_connected+'</td>';
					html += '<td class="connect_state">'+str_connected+'</td>';
				}else{
					html += '<td class="connect_state">&nbsp;</td>';
				}
				// wifi信号强度（弱：0~85，中：86~170，强：171~255）
				if(50<wrssi[i]&&wrssi[i]<=70){
					html += '<td class="signal signal_middle">&nbsp;</td>';
				}else if(70<wrssi[i]&&wrssi[i]<=100){
					html += '<td class="signal signal_Strong">&nbsp;</td>';
				}else{
					html += '<td class="signal">&nbsp;</td>';
				}
				html += '</tr>';
			}catch(e){
				continue;
			}
		}
		html += '</table></div>';
		$("#wifiList").html(html);
		// 设置wifi连接
		$("#wifiList tr").unbind("click");
		$("#wifiList tr").bind("click",function(){
			// 变量索引
			$index = $(this).attr("index");
			// 连接wifi对话框
			$("#connectwifi_msg").html(str_tips_inputwifikey);
			$("#wifikey").val("");
			$("#wifi_ssid").html(wessid[$index]);
			$("#wifi_secret").text(wauth[$index]);
			$("#showkey").attr("checked",false);
			$("#dialog").attr("index",$index);
			$("#dialog").dialog("open");
		});
	});
	return {
		getAjaxData:function(){
			return ajaxData;
		}
	}
}
// 检查key
function checkKey(keytype,keyleng){
/*  some bug exist
	if(keytype.indexOf("WPA") >=0 || keytype.indexOf("WPA2") >=0 ){
		if(keyleng >= 8){
			return true;
		}
	}else if(keytype.indexOf("WEP") >=0 ){
		if(keyleng == 5 || keyleng == 8 ){
			return true;
		}
	}
	return false;
*/
	return true;	
}