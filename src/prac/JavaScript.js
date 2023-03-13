"use strict";
//定义赋值区
let name = "定义形式";
var NumberOne = 100;
var wild;
var intarray = [1,4,2,55,3];
var object = {
    Name: "Time",
    visablity: "invisable",
    value: "priceless",
    Id: "8463",
    flow: function(){
       return Date();
    },
    day: function() {
        let d = new Date().getDay();
        return d;
    }
    /*
    object.Name;
    object["Name"];
    */
};  
var bool = false;
var person = new Object();
person.name = "John";
person.Id = 123124;
var plus = intarray[0] + intarray[3];console.log(plus);





//函数区
function day(){
    let x;
    switch(object.day()){
        case 0: x = "Sunday";break;
        case 1: x = "Monday";break;
        case 2: x = "Tuesday";break;
        case 3: x = "Wendsday";break;
        case 4: x = "Thursday";break;
        case 5: x = "Friday";break;
        case 6: x = "Saturday";break;
    }
    return x;
}
function writeit(){
    window.alert("1+1");
   // alert("1+1=2");
    wild = document.getElementById("change");
    wild.innerHTML = "文字发生了变化";
}
function time(){
    document.getElementById("time").innerHTML = object.flow();
    document.getElementById("day").innerHTML = day();
}
function test(){
    let x = "mix";
    let y = new String("mix");
    document.getElementById("demo").innerHTML = typeof x;
    document.getElementById("demo1").innerHTML = typeof y;
    document.getElementById("demo2").innerHTML = x == y;
    document.getElementById("demo3").innerHTML = x === y;
}
function ted(){
    let x = document.getElementById("ted");
    x.innerHTML = set();
}
function set(){
    let y = new Number();
    let z = 5;
    z = z.toFixed(3);
    return y.constructor.toString().indexOf("Number")  + z;
}
function RE(){
    let x = document.getElementById("re");
    if(x.innerHTML.indexOf("反转") > -1){
        x.innerHTML = "没有哦"; 
    }else{
        x.innerHTML = "反转反转";
    }
    let y = document.getElementById("la");
    if(y.innerHTML.indexOf("反转") > -1){
        y.innerHTML = y.innerHTML.replace(/反转/g,"没有哦");
    }else{
        y.innerHTML = y.innerHTML.replace(/没有哦/g,"反转");
    }
}
function submitit(){
    if(!account(document.getElementById("account").value)){
        alert("账号为九位数字");
        document.getElementById("account").focus();
        return false;
    }
    if(!password(document.getElementById("password").value)){
        alert("密码出现除字母，数字，下划线以外的字符");
        document.getElementById("password").focus();
        return false;
    }
    if(!phonenumber(document.getElementById("phonenumber").value)){
        alert("电话为十一位数字");
        document.getElementById("phonenumber").focus();
        return false;
    }
    alert("成功");
    return flase;
}
function account(acc){
    let result = /^[0-9]{9}$/;
    return result.test(acc);
}
function password(pas){
    let result = /\w/;
    return result.test(pas);
}
function phonenumber(pho){
    let result = /[0-9]{11}/;
    return result.test(pho);
}
/*function addition(add)
{

}
*/
function makesure(){
    let errmessage,numb;
    numb = document.getElementById("numbbb").value;
    errmessage = document.getElementById("result");
    try
    {
        if(numb == "" )
        throw "内容为空";
        if(isNaN(numb))
        throw "请输入数字";
        numb = Number(numb);
        if(numb > 100 || numb < 0)
        throw "输入的数字不符合给出的范围";
        else
        throw "";
    }
    catch(err)
    {
        errmessage.innerHTML = err;
    }
    finally
    {
        document.getElementById("numbbb").value = "";
    }
}
/*function loadXMLDoc()
{
    if(window.XMLHttpRequest)
{
    //  IE7+, Firefox, Chrome, Opera, Safari 浏览器执行代码
    xmlHttp = new XMLHttpRequest();
    console.log(xmlHttp);
    console.log(1);
}
else
{
	// IE6, IE5 浏览器执行代码
    xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
    console.log(xmlHttp);
    console.log(0);
}
    xmlHttp.onreadystatachange=function()
    {
        if(xmlHttp.readyState == 4 && xmlHttp.status == 200)
        {
            document.getElementById("ajax_message").innerHTML = xmlHttp.responseText;
        }
    }
    xmlHttp.open("GET","tool.txt",true);
    xmlHttp.send();
}*/
new Promise(function (resolve,reject){console.log("？？？");resolve("!!!");})
.then(function(value){console.log(value);return "。。。";})
.then(function(value){console.log(value);return 0;})
.then(function(value){console.log(value);throw "123";})
.catch(function(err){console.log(err);})
.finally(setTimeout(function(){console.log("结束");},3000));

async function thatis(){
    setTimeout(function(){console.log(100);},4000);
    setTimeout(function () { console.log(200); }, 5000);
    setTimeout(function(){console.log(300);},6000);
}
async function thisis(){
    /**await后必须跟个Promise */
    var Test = await new Promise(function (resolve,reject){
        resolve("幻影");
    });
    console.log("标志位");
    console.log(Test);
    console.log("标志位");
}
//构造函数调用函数
function give(a, b){
    this.aa = a + b;
    this.bb = a * b;
}
function giveit(){
    var givei = new give(2,3);
    setTimeout(console.log(givei.aa),1000);
    setTimeout(console.log(givei.bb),1000);
}
var tempFunc;
var add = (function () {
    var counter = 0;
    tempFunc = function () { return counter += 1; }
    return tempFunc;
})();
function myFunction() {
    
    console.log("add === tempFunc : " + (add === tempFunc))
    console.log(add());
}
function calculate(a,b){
    var symbol = document.getElementById("symbol").value;
    console.log(a);
    console.log(b);
    console.log(symbol);
    console.log(symbol.toString().indexOf('+'));
    if(symbol.toString().indexOf('+') > -1){
        document.getElementById("res").value = Number (a + b);
    }else{
        if(symbol.indexOf("-") > -1){
            document.getElementById("res").value = Number (a - b);
        }else{
            if(symbol.indexOf("*") > -1){
                document.getElementById("res").value = Number (a * b);
            }else{
                if(symbol.indexOf("/") > -1){
                    document.getElementById("res").value = Number (a / b);
                }
            }
        }
    } 
}
function hid(a){
    var clas = document.getElementsByClassName("hide");
    console.log(clas.length);
    for(var i = 0; i < 5; i++){
        clas[i].style.visibility = "hidden";
    }
}
function such(){
    alert("hahaha");
}
function so(){
    console.log("hiahiahia");
}
function asda(){
    var has = document.getElementById("btn");
    console.log(has);
    has.addEventListener("mouseover",so);
    has.addEventListener("click",such);
}
var hass = document.getElementById("btn2");
hass.addEventListener("click",such);
hass.addEventListener("mouseover",so);

function suuch()
{
    alert("ohhhhhhhhhhhhhh");
}
var test1 = document.getElementById("DIV");
var test2 = document.getElementById("DIV_P");
var test3 = document.getElementById("DIV2");
var test4 = document.getElementById("DIV2_P");
test1.addEventListener("click", such, false);
test2.addEventListener("click", suuch, false);
test3.addEventListener("click", such, true);
test4.addEventListener("click", suuch, true);
test1.addEventListener("mouseover", function(){
    var f = document.getElementById("DIV_P");
    f.innerHTML = "点击查看";
    f.style.color = "snow";
});
test3.addEventListener("mouseover", function(){
    var f = document.getElementById("DIV2_P");
    f.innerHTML = "点击查看";
    f.style.color = "snow";
});
test1.addEventListener("mouseout", function(){
    var f = document.getElementById("DIV_P");
    f.innerHTML = "点击查看";
    f.style.color = "black";
});
test3.addEventListener("mouseout", function(){
    var f = document.getElementById("DIV2_P");
    f.innerHTML = "点击查看";
    f.style.color = "black";
});
