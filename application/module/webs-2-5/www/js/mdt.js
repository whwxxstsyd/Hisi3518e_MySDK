var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

var snapshot_path="/cgi-bin/snapshot.cgi?resolution=720P&user=" + uname + "&pwd=" + upwd;

$(function(){
	var mdtObj=new MDT_MODEL();
	var params=mdtObj.getParams();
	
	$("#md_switch li.switch_icon").click(function(){
		if($(this).hasClass("switch_icon_on")){
			$(this).removeClass("switch_icon_on");
			$(".MDctx").hide();
		}else{
			$(this).addClass("switch_icon_on");
			$(".MDctx").show();
			LoadRegion(params);	
			$("#Sensibility").val(params.alarmSensitivity[0]);		
		}
	});
	loadParams(params);
	$("#save").bind("click",save);
});

function MDT_MODEL(){
	this.getParams=function(){
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?&CFG_alarm&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"alarmEnable":alarm_enb,
					"MdRegion":alarm_bit,
					"alarmSensitivity":alarm_sensitivity
				};
			}
		});
		return res;
	}
	this.setParams=function(obj){
		var res=null;
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
		
		/*****************MD区域***********************/
		var region = new Array(32);
    var k = 0;
    var m = 0;
    
    for (i = 0; i < 12; i++) {
        for (j = 0; j < 20; j++) {
            colbit = GetAlearselect(i, j);
            region[m] |= (colbit << k);
            k++;
            if (k >= 8) {
                k = 0;
                m++;
            }
        }
    }
		for(var i = 0;i<region.length;i++){
			cgiurl += ("&alarm_bit"+i+"="+region[i]);
		}
		if ($("#md_switch li.switch_icon").hasClass("switch_icon_on"))
			cgiurl += "&alarm_enb=on";
		else
			cgiurl += "&alarm_enb=off";
		cgiurl += "&alarm_sensitivity=" + $("#Sensibility").val();
		$.get(cgiurl);
	}
}

function loadParams(params){
	if(params.alarmEnable[0]=="on"){
		$("ul#md_switch li.switch_icon").addClass("switch_icon_on");
		$(".MDctx").show();		
		// 加载覆盖域
		LoadRegion(params);
		//灵敏度
		$("#Sensibility").val(params.alarmSensitivity[0]);
	}
	else
	{
		if($("ul#md_switch li.switch_icon").hasClass("switch_icon_on"))
			$(this).removeClass("switch_icon_on");
		$(".MDctx").hide();			
	}
}

function LoadRegion(params)
{
		var rowno = 0;
		var colno = 0;
		var colbit = 0;
		/*cgi 返回的数组MdRegion转换成行列*/
		for (i = 0; i < params.MdRegion.length && (rowno < 12); i++) {
			for (j = 0; j < 8; j++) {
				colbit = (params.MdRegion[i] >> j) & 1;
				SetAlearselect(rowno, colno, colbit);
				colno++;
				if (colno >= 20){
					//超过一行的个数，则换成下一行
					colno = 0;
					rowno++
				}
				if (rowno >= 12) break;
			}
		}	
}

function save(){
	new MDT_MODEL().setParams();
	save_tips(str_save_succeed);	
}