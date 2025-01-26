
//global variables
var patternArray = [];

//functions
//read URL from selected file and load image
function readURL(input) {
    if (input.files && input.files[0]) {
        var reader = new FileReader();            
        reader.onload = function (e) {
            var img = new Image();
            img.onload = function() {
                getImageInfo(this);
            }
            img.src = e.target.result;
            validateUploadForm();
        }
        reader.readAsDataURL(input.files[0]);
        $("#txtImageName").val(input.files[0].name.replace(".", "_"));
    }
}
//get data from loaded image
function getImageInfo(img) {
    var context = document.getElementById("cvsImage").getContext('2d');
    context.drawImage(img, 0, 0);
    $('#h').text(img.height);
    $('#w').text(img.width);
}
//get image list from server
function getImageList() {
    var loc = "./api/images";
    $.ajax({
        url: loc,
        success : function(data) {              
            var arrayList = data.FilesList;
            
            if (arrayList) {
                //clear list
                $("#cbxImageDelete").html("");
                $("#cbxImageEffect").html("");
                //add blank item first (for delete only)
                $("#cbxImageDelete").append('<option value="" default></option>');
                //add each item from the array
                for (var i=0; i<arrayList.length; i++)
                {
                    //add file to list
                    $("#cbxImageDelete").append('<option value="' + arrayList[i] + '">' + arrayList[i] + '</option>');
                    $("#cbxImageEffect").append('<option value="' + arrayList[i] + '">' + arrayList[i] + '</option>');
                }
            }
            
        }
    });
}
function uploadImage() {
    var context = document.getElementById("cvsImage").getContext('2d');
    //get image data is 0 based (0,0), but second pair is width and height (16,16), not end index (15,15)
    var bytesArray = new Uint8Array(context.getImageData(0, 0, 16, 16).data);
    var imageName = $("#txtImageName").val();
    var imageData = "";                    

    //encode raw data into HEX values
    for (i=0; i<bytesArray.length; i+=4)
    {
        var r = bytesArray[i+0];
        var g = bytesArray[i+1];
        var b = bytesArray[i+2];
        //we dont care about alpha depth
        //var a = bytesArray[i+3];
        imageData += toColor(r) + toColor(g) + toColor(b);
    }

    //stick values in query string
    var loc = "./api/image?imgname=" + imageName + "&imgdata=" + imageData.toUpperCase();

    $.ajax({
        url: loc,
        type: 'PUT',
        processData: false,
        success: function () { 
            //clear forms
            refreshForms();
            //show success message
            $("#txtUploadSuccess").fadeIn("fast", function() {
                setTimeout(function () {$("#txtUploadSuccess").fadeOut("slow");}, 1500);
            });
            //update storage info
            updateStorageInfo();
        }
    });
}
//check if conditions are valid to upload an image
function validateUploadForm() {
    //get pattern of the text control               
    var pattern = $("#txtImageName").attr('pattern');
    var exp = new RegExp('^' + pattern + '$');

    if (exp.test($("#txtImageName").val()) && $("#txtImageName").val().length > 2){
        //text is valid, hide message
        $("#txtInvalidImageName").hide();

        if ($("#btnBrowse").val() != ""){
            //all valid
            $("#btnUploadImage").prop("disabled", false);
        }
        else
        {
            //no file selected
            $("#btnUploadImage").prop("disabled", true);
        }
    }
    else
    {
        //text is not valid, show message, disable upload.
        $("#btnUploadImage").prop("disabled", true);
        //only show message if text is actually entered
        if ($("#txtImageName").val().length > 0)
            $("#txtInvalidImageName").show();
        else
            $("#txtInvalidImageName").hide();
    }
}
function refreshForms() {
    //update lists
    getImageList();
    //Clear upload text
    $("#txtImageName").val("");
    //Clear file selected
    $("#btnBrowse").val("");
    //Clear Pattern
    $("#txtPattern").val("");
    patternArray = [];
    //Set default effect
    updateEffectsControls();
    //clear upload message
    $("#txtUploadSuccess").hide();
    //apply form validation to reset status of buttons
    validateUploadForm();
    //clear the canvas
    var canvas = document.getElementById("cvsImage");
    var context = canvas.getContext('2d');
    context.clearRect(0, 0, canvas.width, canvas.height);
}
function isSelectedDataAttributeTrue(selectName, dataAttr) {
    // Find the select element by name and get the selected option
    var selectedOption = $('#' + selectName).children('option:selected');

    // Check if the specified data attribute of the selected option is set to 'true'
    return selectedOption.data(dataAttr) === true;
}
function effectHasColor()
{
    return isSelectedDataAttributeTrue("cbxEffect", "col");
}
function effectHasImage()
{
    return isSelectedDataAttributeTrue("cbxEffect", "img");
}
function effectHasPattern()
{
    return isSelectedDataAttributeTrue("cbxEffect", "pattrn");
}
function updateEffectsControls() {
    $("#colSolid").hide();
    $("#cbxImageEffect").hide();
    $("#txtSetEffectError").hide();
    $("#txtSetEffectSuccess").hide();
    $("#grpPattern").hide();

    if (effectHasImage())
        $("#cbxImageEffect").show();
    else if (effectHasColor())
        $("#colSolid").show();
    else if (effectHasPattern())
        $("#grpPattern").show();
}
function updateStorageInfo() {
    $.ajax({
        url: "./api/storage",
        success: function (data) { 
            var used = data.UsedBytes / 1024 / 1024;
            var total = data.TotalBytes / 1024 / 1024;
            var pct = Math.round(used / total * 100);
            if (pct < 1)
                pct = 1;
            else if (pct > 100)
                pct = 100;

            //show info
            $("#proUsedSpace").val(pct);
            $("#txtTotalSpace").text(total.toFixed(2));
            $("#txtUsedSpace").text(used.toFixed(2));
            $("#txtPctUsedSpace").text(pct + "%");
        }
    });
}
function updateDeviceInfo()
{
    $.ajax({
        url: "./api/info",
        success: function (data) {
            var hostname = data.device.hostname;
            var ip = data.device.ip;
            var firmware = data.device.firmware;
            var signal = data.device.signal;
            var ssid = data.device.ssid;

            //show info
            updateSignalStrength(signal);
            $("#txtHostname").text(hostname.toUpperCase());
            $("#txtIP").text(ip);
            $("#txtVersion").text(firmware);
            $("#txtSSID").text(ssid);
        }
    });
}
function updateSignalStrength(dBm)
{
    if (dBm > -30)
    {
        //excellent
        $("#txtdBm").html("<i class='bi bi-wifi' alt='Excellent'></i>");
    }
    else if (dBm > -67)
    {
        //good
        $("#txtdBm").html("<i class='bi bi-wifi' alt='Good'></i>");
    }
    else if (dBm > -70)
    {
        //ok
        $("#txtdBm").html("<i class='bi bi-wifi-2'  alt='OK'></i>");
    }
    else if (dBm > -80)
    {
        //passable
        $("#txtdBm").html("<i class='bi bi-wifi-1'  alt='Passable'></i>");
    }
    else
    {
        //horrible
        $("#txtdBm").html("<i class='bi bi-wifi-off'  alt='Poor'></i>");
    }
}
function updateCurrentEffect()
{
    $.ajax({
        url: "./api/effect",
        success: function (data) { 
            var used = data.UsedBytes / 1024 / 1024;
            var total = data.TotalBytes / 1024 / 1024;
            var pct = Math.round(used / total * 100);
            if (pct < 1)
                pct = 1;
            else if (pct > 100)
                pct = 100;

            //show info
            $("#cbxEffect").val(data.effect.toLowerCase());
            $("#sldBrightness").val(data.brightness);
        }
    });                
}
function addPatternPixel(item) {
    //maintain space after * so the line wraps
    let pixel = $("<span>", {"text":"* ", "style":"color: #"+item});
    $("#ctlPatternDisplay").append(pixel);
}
function updatePatternData() {
    //update array
    $("#txtPattern").val(patternArray.join(""));
    
    //Clear control
    $("#ctlPatternDisplay").html("");

    //Re-populate control
    patternArray.forEach(addPatternPixel);
}
function toColor(n) {
    if (n<16)
        return "0" + n.toString(16);
    else
        return n.toString(16);
}
//ready
$(function() {
    //Send Effect command
    $("#btnEffect").on("click", function() {
        var loc = "./api/effect";
        var dat = {};

        //choose which parameters to send
        if (effectHasImage())
            dat = {name: $("#cbxEffect").val(), imgname: $("#cbxImageEffect").val()};                    
        else if (effectHasColor())
            dat = {name: $("#cbxEffect").val(), color: $("#colSolid").val().replace("#", "")};
        else if (effectHasPattern())
            dat = {name: $("#cbxEffect").val(), color: $("#txtPattern").val()};
        else
            dat = {name: $("#cbxEffect").val()};

        if ($("#chkDefaultEffect").val() === "on")
            dat.setdefault = 1;
        else
            dat.setdefault = 0;
        
        $.ajax({
            url: loc,
            type: 'PUT',
            data: dat,
            success: function (data) {
                //show success message
                $("#txtSetEffectSuccess").fadeIn("fast", function() {
                    $("#txtSetEffectSuccessDescription").html(data);
                    setTimeout(function () {$("#txtSetEffectSuccess").fadeOut("slow");}, 1500);
                });
            },
            error: function (e, msg) {
                //show error message
                $("#txtSetEffectError").fadeIn("fast", function() {
                    $("#txtSetEffectErrorDescription").html(msg);
                    setTimeout(function () {$("#txtSetEffectError").fadeOut("slow");}, 1500);
                });
            }
        });
    });
    $("#cbxEffect").change(function (){
        updateEffectsControls();
    });
    //Send brightness command
    $("#btnBrightness").on("click", function() {
        var i = parseInt($("#sldBrightness").val());
        var loc = "./api/effect";
        var dat = {brightness: i.toString(16)};
        $.ajax({
            url: loc,
            data: dat
        });
    });
    //Brightness UI feedback
    $("#sldBrightness").on("input", function() {
        var i = parseInt($("#sldBrightness").val());
        //$("#txtBrightness").text(i.toString(16).toUpperCase());
        //display in decimal instead with cap to 100 (%)
        $("#txtBrightness").text(i.toString() + "%");
    });
    //File select button
    $("#btnBrowse").change(function(){
        readURL(this);
    });
    //Image name
    $("#txtImageName").on("keyup", function(){
        validateUploadForm();
    });
    //Send Image command
    $("#btnUploadImage").on("click", function() {
        uploadImage();
    });
    //Image to be deleted selected
    $("#cbxImageDelete").change(function(){
        if ($("#cbxImageDelete").val() == "")
        {
            $("#btnDeleteImage").prop("disabled", true);
        }
        else
        {
            $("#btnDeleteImage").prop("disabled", false);
        }
    });
    //Delete image button
    $("#btnDeleteImage").on("click", function(){
        //get image name from drop-down
        var imageName = $("#cbxImageDelete").val(); 

        //stick values in query string
        var loc = "./api/image?imgname=" + imageName;

        $.ajax({
            url: loc,
            type: 'DELETE',
            processData: false,
            success: function () { refreshForms(); updateStorageInfo(); }
        });
    });
    //Add pattern button
    $("#btnAddPattern").on("click", function(){
        patternArray.push($("#colPattern").val().replace("#", ""));
        updatePatternData();
    });
    //Remove pattern button
    $("#btnRemovePattern").on("click", function(){
        patternArray.pop();
        updatePatternData();
    });                

    //enable popover
    $('[data-toggle="popover"]').popover(); 

    //update device information
    updateDeviceInfo();
    //get form ready
    refreshForms();
    //get storage info
    updateStorageInfo();
    //get current effect
    updateCurrentEffect();
});
