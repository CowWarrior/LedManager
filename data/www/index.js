var patternArray=[];function readURL(input){if(input.files&&input.files[0]){var reader=new FileReader();reader.onload=function(e){var img=new Image();img.onload=function(){getImageInfo(this);}
img.src=e.target.result;validateUploadForm();}
reader.readAsDataURL(input.files[0]);$("#txtImageName").val(input.files[0].name.replace(".","_"));}}
function getImageInfo(img){var context=document.getElementById("cvsImage").getContext('2d');context.drawImage(img,0,0);$('#h').text(img.height);$('#w').text(img.width);}
function getImageList(){var loc="./api/images";$.ajax({url:loc,success:function(data){var arrayList=data.FilesList;if(arrayList){$("#cbxImageDelete").html("");$("#cbxImageEffect").html("");$("#cbxImageDelete").append('<option value="" default></option>');for(var i=0;i<arrayList.length;i++)
{$("#cbxImageDelete").append('<option value="'+arrayList[i]+'">'+arrayList[i]+'</option>');$("#cbxImageEffect").append('<option value="'+arrayList[i]+'">'+arrayList[i]+'</option>');}}}});}
function uploadImage(){var context=document.getElementById("cvsImage").getContext('2d');var bytesArray=new Uint8Array(context.getImageData(0,0,16,16).data);var imageName=$("#txtImageName").val();var imageData="";for(i=0;i<bytesArray.length;i+=4)
{var r=bytesArray[i+0];var g=bytesArray[i+1];var b=bytesArray[i+2];imageData+=toColor(r)+toColor(g)+toColor(b);}
var loc="./api/image?imgname="+imageName+"&imgdata="+imageData.toUpperCase();$.ajax({url:loc,type:'PUT',processData:false,success:function(){refreshForms();$("#txtUploadSuccess").fadeIn("fast",function(){setTimeout(function(){$("#txtUploadSuccess").fadeOut("slow");},1500);});updateStorageInfo();}});}
function validateUploadForm(){var pattern=$("#txtImageName").attr('pattern');var exp=new RegExp('^'+pattern+'$');if(exp.test($("#txtImageName").val())&&$("#txtImageName").val().length>2){$("#txtInvalidImageName").hide();if($("#btnBrowse").val()!=""){$("#btnUploadImage").prop("disabled",false);}
else
{$("#btnUploadImage").prop("disabled",true);}}
else
{$("#btnUploadImage").prop("disabled",true);if($("#txtImageName").val().length>0)
$("#txtInvalidImageName").show();else
$("#txtInvalidImageName").hide();}}
function refreshForms(){getImageList();$("#txtImageName").val("");$("#btnBrowse").val("");$("#txtPattern").val("");patternArray=[];updateEffectsControls();$("#txtUploadSuccess").hide();validateUploadForm();var canvas=document.getElementById("cvsImage");var context=canvas.getContext('2d');context.clearRect(0,0,canvas.width,canvas.height);}
function isSelectedDataAttributeTrue(selectName,dataAttr){var selectedOption=$('#'+selectName).children('option:selected');return selectedOption.data(dataAttr)===true;}
function effectHasColor()
{return isSelectedDataAttributeTrue("cbxEffect","col");}
function effectHasImage()
{return isSelectedDataAttributeTrue("cbxEffect","img");}
function effectHasPattern()
{return isSelectedDataAttributeTrue("cbxEffect","pattrn");}
function updateEffectsControls(){$("#colSolid").hide();$("#cbxImageEffect").hide();$("#txtSetEffectError").hide();$("#txtSetEffectSuccess").hide();$("#grpPattern").hide();if(effectHasImage())
$("#cbxImageEffect").show();else if(effectHasColor())
$("#colSolid").show();else if(effectHasPattern())
$("#grpPattern").show();}
function updateStorageInfo(){$.ajax({url:"./api/storage",success:function(data){var used=data.UsedBytes/1024/1024;var total=data.TotalBytes/1024/1024;var pct=Math.round(used/total*100);if(pct<1)
pct=1;else if(pct>100)
pct=100;$("#proUsedSpace").val(pct);$("#txtTotalSpace").text(total.toFixed(2));$("#txtUsedSpace").text(used.toFixed(2));$("#txtPctUsedSpace").text(pct+"%");}});}
function updateDeviceInfo()
{$.ajax({url:"./api/info",success:function(data){var hostname=data.device.hostname;var ip=data.device.ip;var firmware=data.device.firmware;var signal=data.device.signal;var ssid=data.device.ssid;updateSignalStrength(signal);$("#txtHostname").text(hostname.toUpperCase());$("#txtIP").text(ip);$("#txtVersion").text(firmware);$("#txtSSID").text(ssid);}});}
function updateSignalStrength(dBm)
{if(dBm>-30)
{$("#txtdBm").html("<i class='bi bi-wifi' alt='Excellent'></i>");}
else if(dBm>-67)
{$("#txtdBm").html("<i class='bi bi-wifi' alt='Good'></i>");}
else if(dBm>-70)
{$("#txtdBm").html("<i class='bi bi-wifi-2'  alt='OK'></i>");}
else if(dBm>-80)
{$("#txtdBm").html("<i class='bi bi-wifi-1'  alt='Passable'></i>");}
else
{$("#txtdBm").html("<i class='bi bi-wifi-off'  alt='Poor'></i>");}}
function updateCurrentEffect()
{$.ajax({url:"./api/effect",success:function(data){var used=data.UsedBytes/1024/1024;var total=data.TotalBytes/1024/1024;var pct=Math.round(used/total*100);if(pct<1)
pct=1;else if(pct>100)
pct=100;$("#cbxEffect").val(data.effect.toLowerCase());$("#sldBrightness").val(data.brightness);}});}
function addPatternPixel(item){let pixel=$("<span>",{"text":"* ","style":"color: #"+item});$("#ctlPatternDisplay").append(pixel);}
function updatePatternData(){$("#txtPattern").val(patternArray.join(""));$("#ctlPatternDisplay").html("");patternArray.forEach(addPatternPixel);}
function toColor(n){if(n<16)
return"0"+n.toString(16);else
return n.toString(16);}
$(function(){$("#btnEffect").on("click",function(){var loc="./api/effect";var dat={};if(effectHasImage())
dat={name:$("#cbxEffect").val(),imgname:$("#cbxImageEffect").val()};else if(effectHasColor())
dat={name:$("#cbxEffect").val(),color:$("#colSolid").val().replace("#","")};else if(effectHasPattern())
dat={name:$("#cbxEffect").val(),color:$("#txtPattern").val()};else
dat={name:$("#cbxEffect").val()};if($("#chkDefaultEffect").val()==="on")
dat.setdefault=1;else
dat.setdefault=0;$.ajax({url:loc,type:'PUT',data:dat,success:function(data){$("#txtSetEffectSuccess").fadeIn("fast",function(){$("#txtSetEffectSuccessDescription").html(data);setTimeout(function(){$("#txtSetEffectSuccess").fadeOut("slow");},1500);});},error:function(e,msg){$("#txtSetEffectError").fadeIn("fast",function(){$("#txtSetEffectErrorDescription").html(msg);setTimeout(function(){$("#txtSetEffectError").fadeOut("slow");},1500);});}});});$("#cbxEffect").change(function(){updateEffectsControls();});$("#btnBrightness").on("click",function(){var i=parseInt($("#sldBrightness").val());var loc="./api/effect";var dat={brightness:i.toString(16)};$.ajax({url:loc,data:dat});});$("#sldBrightness").on("input",function(){var i=parseInt($("#sldBrightness").val());$("#txtBrightness").text(i.toString()+"%");});$("#btnBrowse").change(function(){readURL(this);});$("#txtImageName").on("keyup",function(){validateUploadForm();});$("#btnUploadImage").on("click",function(){uploadImage();});$("#cbxImageDelete").change(function(){if($("#cbxImageDelete").val()=="")
{$("#btnDeleteImage").prop("disabled",true);}
else
{$("#btnDeleteImage").prop("disabled",false);}});$("#btnDeleteImage").on("click",function(){var imageName=$("#cbxImageDelete").val();var loc="./api/image?imgname="+imageName;$.ajax({url:loc,type:'DELETE',processData:false,success:function(){refreshForms();updateStorageInfo();}});});$("#btnAddPattern").on("click",function(){patternArray.push($("#colPattern").val().replace("#",""));updatePatternData();});$("#btnRemovePattern").on("click",function(){patternArray.pop();updatePatternData();});$('[data-toggle="popover"]').popover();updateDeviceInfo();refreshForms();updateStorageInfo();updateCurrentEffect();});