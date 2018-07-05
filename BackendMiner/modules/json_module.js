//json_module.js 
//create on 2018-05-16 by wjun


//json-->string
function BwJsonStringify(jsonString)
{
	var jsonObj = JSON.stringify(jsonString);
	return jsonObj;
}
  
//string-->json
function BwJsonParse(jsonObj)
{
	var jsonArr = JSON.parse(jsonObj);
	return jsonArr;
}
  

exports.BwJsonStringify = BwJsonStringify;
exports.BwJsonParse = BwJsonParse;
