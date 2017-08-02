var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){
	var obj=getDeviceInfo();

	$("#git_header").text(obj.git_head);	
	$("#fm_version").text(obj.soft_version);
	$("#web_version").text(web_version);	
	$("#dev_model").val(obj.camera_type);
	$("#oem_info").val(obj.oem_info);
	$("#p2p_type").val(obj.p2p_type);	
	$("#uid").val(obj.uid);
	$("#inner_user").val(obj.inner_user);
	$("#inner_pwd").val(obj.inner_pwd);	
	$("#limit_x").val(obj.limit_x);
	$("#limit_y").val(obj.limit_y);	
	$("#region").val(obj.region);
	$("#description_file").val(obj.description_file);
	$("#save").bind("click", set_value);	
});

function getDeviceInfo(){
	var res=null;
	var cgiurl="/cgi-bin/get_param.cgi?user="+uname+"&pwd="+upwd;
	cgiurl+="&git_head&soft_version&camera_type&oem_info&uid&p2p_type&inner_user&inner_pwd&limit_x&limit_y&region&description_file";
	$.ajax({
		url:cgiurl,
		async:false,
		cache:false,
		success: function(data){
			eval(data);
			res={
				"git_head":git_head,
				"soft_version":soft_version,
				"camera_type":camera_type,
				"oem_info":oem_info,
				"p2p_type":p2p_type,
				"uid":uid,
				"inner_user":inner_user,
				"inner_pwd":inner_pwd,				
				"limit_x":limit_x,
				"limit_y":limit_y,
				"region":region,
				"description_file":description_file,
			};
		}
	});
	return res;
}

function set_value(){
		var cgiurl="/cgi-bin/set_param.cgi?user="+uname+"&pwd="+upwd;
		cgiurl+="&camera_type="+$("#dev_model").val();
		cgiurl+="&oem_info="+$("#oem_info").val();
		cgiurl+="&p2p_type="+$("#p2p_type").val();
		cgiurl+="&uid="+$("#uid").val();
		cgiurl+="&inner_user="+$("#inner_user").val();
		cgiurl+="&inner_pwd="+$("#inner_pwd").val();
		cgiurl+="&limit_x="+$("#limit_x").val();
		cgiurl+="&limit_y="+$("#limit_y").val();		
		cgiurl+="&region="+$("#region").val();
		cgiurl+="&online_des_file="+$("#online_des_file").val();		
		$.get(cgiurl);
		
	save_tips(str_save_succeed);	
}