var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){
	// 加载时间表
	loadTimeSchedule();
	// 初始化参数
	initParams();
	// 录像开关切换
	$("#timingRecord_switch li.switch_icon").click(function(){
		if($(this).hasClass("switch_icon_on")){
			$(this).removeClass("switch_icon_on");
			$(".recordSettings").hide();
		}else{
			$(this).addClass("switch_icon_on");
			$(".recordSettings").show();
		}
	});
	// 录像文件时长改变
	$("#videoDurationSel").change(function(){
		$val = $("#videoDurationSel").val();
		if($val){
			$("#videoDuration").css("display","none");
			$("#videoDuration").next().css("display","none");
		}else{
			$("#videoDuration").css("display","inline-block");
			$("#videoDuration").next().css("display","inline-block");
			$("#videoDuration").focus();
		}
		$("#videoDuration").val($val);
	});
	// 选择所有天
	$("#timeSchedule td[data='week']").click(function(){
		if($(this).hasClass("selectRow")){
			$(this).removeClass("selectRow");
			$(this).siblings("td").removeClass("selected");		
		}else{
			$(this).addClass("selectRow");
			$(this).siblings("td").addClass("selected");
		}
	});
	//全选
	$("#timeSchedule td:eq(0)").click(function(){
		if($(this).hasClass("selectRow")){
			$(this).removeClass("selectRow");$(this).parent("tr").nextAll("tr").each(function(){
				$(this).children("td:gt(0)").removeClass("selected");
			});
			
		}else{
			$(this).addClass("selectRow");
			$(this).parent("tr").nextAll("tr").each(function(){
				$(this).children("td:gt(0)").addClass("selected");
			});
		}
	});
	// 选择每天的同一时间
	$("#timeSchedule td[data='hours']").click(function(){
		var index = $(this).index();
		if($(this).hasClass("selectRow")){
			$(this).removeClass("selectRow");
			$("#timeSchedule tr:gt(0)").each(function(){
				$(this).children("td:eq("+(2*index-1)+")").removeClass("selected");
				$(this).children("td:eq("+(2*index)+")").removeClass("selected");
			});
		}else{
			$(this).addClass("selectRow");
			$("#timeSchedule tr:gt(0)").each(function(){
				$(this).children("td:eq("+(2*index-1)+")").addClass("selected");
				$(this).children("td:eq("+(2*index)+")").addClass("selected");
			});
		}
	});
	$("#timeSchedule td[data='timeSlot']").click(function(){
		if($(this).hasClass("selected")){
			$(this).removeClass("selected");
		}else{
			$(this).addClass("selected");
		}
	});
	var flag;
	$("#timeSchedule td[data='timeSlot']").mousedown(function(){
		flag = true;
	});
	$("#timeSchedule td[data='timeSlot']").mouseup(function(){
		flag = false;
	});
	$("#timeSchedule td[data='timeSlot']").mouseover(function(){
		if(flag){
			if($(this).hasClass("selected")){
				$(this).removeClass("selected");
			}else{
				$(this).addClass("selected");
			}
		}
	});
	$("#save").bind("click",save);
});
// 绑定参数值
function initParams(){
	var recordObj=new RECORD_MODEL();
	var params=recordObj.getParams();
	
	// 定时录像开关
	if(params.recordEnable == "on"){
		$("#timingRecord_switch li.switch_icon").addClass("switch_icon_on");
		$(".recordSettings").show();
	}else{
		$(".recordSettings").hide();
	}
	// 录像文件分辨率（码率）
	$("#bitRateSel").val(params.recordResolution);
	// 录像文件时长
	$("#videoDurationSel").val(params.recordPeroid);
	// 时间表
	for(var w=0;w<7;w++){
		$("#timeSchedule tr:eq("+(w+1)+")").children("td:gt(0)").each(function(index){
			if(eval("params.recordSch["+w+"]").substr(index,1) == "1"){
				$(this).addClass("selected");
			}
		});
	}
}
// 加载时间表
function loadTimeSchedule(){
	var weekArray = new Array();
	weekArray[0] = str_sunday;
	weekArray[1] = str_monday;
	weekArray[2] = str_tuesday;
	weekArray[3] = str_wednesday;
	weekArray[4] = str_thursday;
	weekArray[5] = str_friday;
	weekArray[6] = str_saturday;
	var html = "<table>";
	// 表格title
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
	// 表格内容
	for(var row=0;row<7;row++){
		html += "<tr><td data='week'>"+weekArray[row]+"</td>";
		for(var col=0;col<24;col++){
			for(var i=0;i<2;i++){
				html += "<td data='timeSlot' title='"+weekArray[row]+":"+(i==0?+col+':00~'+col+':30':+col+':30~'+(col+1)+':00')+"'>&nbsp;</td>";
			}
		}
		html += "</tr>";
	}
	html += "</table>";
	$("#timeSchedule").html(html);
}

function RECORD_MODEL(){
	this.getParams=function(){
		var res=null;
		var cgiurl="/cgi-bin/get_param.cgi?&CFG_rec&user="+uname+"&pwd="+upwd;
		$.ajax({
			url:cgiurl,
			async:false,
			cache:false,
			success: function(data){
				eval(data);
				res={
					"recordEnable":rec_enb,
					"recordResolution":rec_resolution,
					"recordPeroid":rec_peroid,
					"recordSch":rec_sch,
				};
			}
		});
		return res;
	}
	
	this.setParams=function(obj){
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
		
		if($("#timingRecord_switch li.switch_icon").hasClass("switch_icon_on")){
			cgiurl += "&rec_enb=" + "on";
		}else{
			cgiurl += "&rec_enb=" + "off";
		}
		cgiurl += "&rec_resolution=" + $("#bitRateSel").val();
		cgiurl += "&rec_peroid=" + $("#videoDurationSel").val();
		for(var w=0;w<7;w++){
			var week = "";
			$("#timeSchedule tr:eq("+(w+1)+")").children("td:gt(0)").each(function(index){
				if($(this).hasClass("selected")){
					week +="1";	
				}else{
					week +="0";
				}
			});
			cgiurl += "&rec_sch"+w+"="+week;
		}

		$.get(cgiurl);
	}
}

function save(){
	new RECORD_MODEL().setParams();
	save_tips(str_save_succeed);	
}