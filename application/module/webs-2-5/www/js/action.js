var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){
	loadParams();
	$("#save").bind("click",save);
});

function ALARM_MODEL(){
	this.getParams=function(){
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?CFG_alarm&http_port&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"emailAlarm":alarm_email_pic,
					"ftpAlarm":alarm_ftp_pic,
					"audioAlarm":alarm_sound,					
					"picToSD":alarm_pic_sd,
					"recToSD":alarm_rec_sd,
					"picCount":alarm_pic_cnt,
					"scheduleVals":alarm_sch
				};
			}
		});
		return res;
	}
	this.setParams=function(obj){
		var res=null;
		var params="user="+uname+"&pwd="+upwd;
		params+="&alarm_email_pic="+obj.emailAlarm;
		params+="&alarm_sound="+obj.audioAlarm;
		params+="&alarm_ftp_pic="+obj.ftpAlarm;
		params+="&alarm_pic_sd="+obj.picToSD;
		params+="&alarm_rec_sd="+obj.recToSD;
		params+="&alarm_pic_cnt="+obj.picCount;
		for(var i=0;i<obj.scheduleVals.length;i++){
			params+="&alarm_sch"+i+"="+obj.scheduleVals[i];
		}
		$.get("/cgi-bin/set_param.cgi",params);
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
	var alarmObj=new ALARM_MODEL();
	var params=alarmObj.getParams();
	if(params.emailAlarm=="on"){
		$("ul#emailAlarm_switch li.switch_icon").addClass("switch_icon_on");
	}
	if(params.ftpAlarm=="on"){
		$("ul#ftpAlarm_switch li.switch_icon").addClass("switch_icon_on");
	}
	if(params.audioAlarm=="on"){
		$("ul#audioAlarm_switch li.switch_icon").addClass("switch_icon_on");
	}
	if(params.picToSD=="on"){
		$("ul#saveImgToSD_switch li.switch_icon").addClass("switch_icon_on");
	}
	if(params.recToSD=="on"){
		$("ul#saveVideoToSD_switch li.switch_icon").addClass("switch_icon_on");
	}
	$("#snapCount").val(params.picCount);
	loadSchedule("#alarmSchedule");
	setScheduleValues("#alarmSchedule",params.scheduleVals);
}

//加载时间表
function loadSchedule(id){
	var weekArray = new Array();
	weekArray[0] = str_sunday;
	weekArray[1] = str_monday;
	weekArray[2] = str_tuesday;
	weekArray[3] = str_wednesday;
	weekArray[4] = str_thursday;
	weekArray[5] = str_friday;
	weekArray[6] = str_saturday;
	var html = '<table class="schedule">';
	html += "<tr><td>"+str_selectAll+"</td>";
	for(var i=0;i<24;i++){
		html += "<td colspan='2' data='hours'>";
		if(i<=9){
			html += "0"+i;
		}else{
			html += i;
		}
		html += "</td>";
	}
	html += "</tr>";
	for(var row=0;row<7;row++){
		html += "<tr><td data='week'>"+weekArray[row]+"</td>";
		for(var col=0;col<24;col++){
			for(var i=0;i<2;i++){
				html += "<td data='timeSlot' title='"+weekArray[row]+":"+(i==0?+col+':00~'+col+':30':+col+':30~'+(col==23?"00":col+1)+':00')+"'>&nbsp;</td>";
			}
		}
		html += "</tr>";
	}
	html += "</table>";
	$(id).html(html);
	$(id+" td[data='week']").click(function(){
		if($(this).hasClass("selectRow")){
			$(this).removeClass("selectRow");
			$(this).siblings("td").removeClass("selected");		
		}else{
			$(this).addClass("selectRow");
			$(this).siblings("td").addClass("selected");
		}
	});
	$(id+" td:eq(0)").click(function(){
		if($(this).hasClass("selectAll")){
			$(id+" td[data='week']").removeClass("selectRow");
			$(id+" td[data='hours']").removeClass("selectRow");
			$(this).removeClass("selectAll");
			$(this).parent("tr").nextAll("tr").each(function(){
				$(this).children("td:gt(0)").removeClass("selected");
			});
		}else{
			$(id+" td[data='week']").addClass("selectRow");
			$(id+" td[data='hours']").addClass("selectRow");
			$(this).addClass("selectAll");
			$(this).parent("tr").nextAll("tr").each(function(){
				$(this).children("td:gt(0)").addClass("selected");
			});
		}
	});
	$(id+" td[data='hours']").click(function(){
		var index = $(this).index();
		if($(this).hasClass("selectRow")){
			$(this).removeClass("selectRow");
			$(id+" tr:gt(0)").each(function(){
				$(this).children("td:eq("+(2*index-1)+")").removeClass("selected");
				$(this).children("td:eq("+(2*index)+")").removeClass("selected");
			});
		}else{
			$(this).addClass("selectRow");
			$(id+" tr:gt(0)").each(function(){
				$(this).children("td:eq("+(2*index-1)+")").addClass("selected");
				$(this).children("td:eq("+(2*index)+")").addClass("selected");
			});
		}
	});
	$(id+" td[data='timeSlot']").click(function(){
		if($(this).hasClass("selected")){
			$(this).removeClass("selected");
		}else{
			$(this).addClass("selected");
		}
	});
	var flag = false;
	$(id+" td[data='timeSlot']").mousedown(function(){
		flag = true;
	});
	$(id+" td[data='timeSlot']").mouseup(function(){
		flag = false;
	});
	$(id+" td[data='timeSlot']").mouseover(function(){
		if(flag){
			if($(this).hasClass("selected")){
				$(this).removeClass("selected");
			}else{
				$(this).addClass("selected");
			}
		}
	});
}

//获取时间表的值
function getScheduleValues(id){
	var weekvalues=new Array(7);
	for(var w=0;w<7;w++){
		var weekval = "";
		$(id+" tr:eq("+(w+1)+")").children("td:gt(0)").each(function(index){
			if($(this).hasClass("selected")){
				weekval +="1";	
			}else{
				weekval +="0";
			}
		});
		weekvalues[w]=weekval;
	}
	return weekvalues;
}

//设置时间表的值(some error)
function setScheduleValues(id,vals){
	var selectAllflag = true;
	for(var w=0;w<7;w++){
		$(id+" tr:eq("+(w+1)+")").children("td:gt(0)").each(function(index){
			if(vals[w].substr(index,1) == "1"){
				$(this).addClass("selected");
			}else{
				selectAllflag = false;
			}
		});	
	}
	if(selectAllflag){
		$(id+" td:eq(0)").addClass("selectAll");
		$(id+" td[data='week'],"+id+" td[data='hours']").addClass("selectRow");
	}
}

function save(){
	var obj=new Object();
	obj.emailAlarm=$("ul#emailAlarm_switch li.switch_icon").hasClass("switch_icon_on")?"on":"off";
	obj.ftpAlarm=$("ul#ftpAlarm_switch li.switch_icon").hasClass("switch_icon_on")?"on":"off";
	obj.audioAlarm=$("ul#audioAlarm_switch li.switch_icon").hasClass("switch_icon_on")?"on":"off";	
	obj.picToSD=$("ul#saveImgToSD_switch li.switch_icon").hasClass("switch_icon_on")?"on":"off";
	obj.recToSD=$("ul#saveVideoToSD_switch li.switch_icon").hasClass("switch_icon_on")?"on":"off";
	obj.picCount=$("#snapCount").val();
	obj.scheduleVals=getScheduleValues("#alarmSchedule");
	new ALARM_MODEL().setParams(obj);
	save_tips(str_save_succeed);	
}
