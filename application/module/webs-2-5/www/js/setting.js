// JavaScript Document

var uname=Base64.decode(getCookie("username"));
var upwd=Base64.decode(getCookie("password"));

$(function(){
	$("#menu dt").click(function(){
		$(this).parent("dl").siblings("dl").children("dd").slideUp(100);
		var $dd=$(this).siblings("dd");
		if($dd.css("display")=="none"){
			$dd.slideDown(100);
//			$(this).next("dd").children("a").click();
		}else{
			$dd.slideUp(100);
		}
		/* 隐藏列表 */
		if (with_multi_cam == 0)
			$("#widedev_page").hide();
		if (with_ptz == 0)
			$("#ptz_page").hide();
		if (with_audio == 0)
			$("#audio_page").hide();	
		if ((with_multi_cam == 0) && (with_ptz == 0))
			$("#other_setting").hide();		
	});
		$("#menu dd a").click(function(){
		$(this).parent("dd").siblings("dd").children("a").css({"font-weight":"normal","color":"#666"});
		$(this).css({"font-weight":"bold","color":"rgb(83, 168, 214)"});
		var $href = $(this).attr('href');
		$("#ife_ctx").attr('src',$href);
	
		/* 隐藏列表 */
		if (with_multi_cam == 0)
			$("#widedev_page").hide();
		if (with_ptz == 0)
			$("#ptz_page").hide();
		if (with_audio == 0)
			$("#audio_page").hide();	
		if ((with_multi_cam == 0) && (with_ptz == 0))
			$("#other_setting").hide();
			
		return false;
	});
	
	if ((with_multi_cam == 0) && (with_ptz == 0))
		$("#other_setting").hide();	
});