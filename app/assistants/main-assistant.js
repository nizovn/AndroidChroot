function MainAssistant() {
};

MainAssistant.prototype.setup = function() {
	this.controller.get('main-title').innerHTML = $L('AndroidChroot');
	this.controller.get('version').innerHTML = $L("v" + Mojo.Controller.appInfo.version);
	this.controller.get('subTitle').innerHTML = $L('Android in card project');	
	this.attributes = {
		type : Mojo.Widget.activityButton
	}
	this.buttonmodelstart = {
		label : "Start android && Start client",
		buttonClass : "affirmative",
		disabled: false
	};
	this.controller.setupWidget("StartButton", this.attributes, this.buttonmodelstart);
	this.buttonmodelstop = {
		label : "Send shutdown",
		buttonClass : "negative",
		disabled: true
	};
	this.controller.setupWidget("StopButton", this.attributes, this.buttonmodelstop);
	this.textfieldmodel = {
		disabled: true,
		visible:false,
		value:""
	};
	this.controller.setupWidget("textFieldInfo",
		this.attributes = {
			multiline: true,
			enterSubmits: false,
			autoFocus: false,
			visible:false
		},
		this.textfieldmodel
	);
	Mojo.Event.listen(this.controller.get("StartButton"),Mojo.Event.tap, this.TapStart.bind(this));
	Mojo.Event.listen(this.controller.get("StopButton"),Mojo.Event.tap, this.TapStop.bind(this));
};
MainAssistant.prototype.TapStart = function(event) {
	this.controller.get('StartButton').mojo.deactivate();
	if((mount==false)&&(client==false)){
		this.buttonmodelstart.label= "Start client";
		this.buttonmodelstart.buttonClass= "affirmative";
		this.buttonmodelstart.disabled= true;
		this.buttonmodelstop.label= "Send shutdown";
		this.buttonmodelstop.buttonClass= "negative";
		this.buttonmodelstop.disabled= true;
		this.controller.modelChanged(this.buttonmodelstart);
		this.controller.modelChanged(this.buttonmodelstop);
		this.controller.get('StartButton').mojo.activate();
		this.controller.serviceRequest('palm://com.nizovn.androidchroot.main.c', {
						method: "setstate",
						parameters: {"state": "on"},
						onSuccess: this.SuccessOn.bind(this),
						onFailure: function(){}
					});
	};
	if((mount==true)&&(client==false)){
		this.buttonmodelstart.label= "Start client";
		this.buttonmodelstart.buttonClass= "affirmative";
		this.buttonmodelstart.disabled= true;
		this.buttonmodelstop.label= "Send shutdown";
		this.buttonmodelstop.buttonClass= "negative";
		this.buttonmodelstop.disabled= true;
		this.controller.modelChanged(this.buttonmodelstart,this);
		this.controller.modelChanged(this.buttonmodelstop,this);
		this.StartClient();
	};
};
MainAssistant.prototype.TapStop = function(event) {
	this.controller.get('StopButton').mojo.deactivate();
	if((mount==true)&&(client==true)){
		this.controller.serviceRequest('palm://com.nizovn.androidchroot.main.c', {
						method: "setstate",
						parameters: {"state": "off"},
						onSuccess: function(){},
						onFailure: function(){}
					});
	};
}
MainAssistant.prototype.Failure = function(event) {
	Mojo.log("getStatus failure, results=" + JSON.stringify(event));
};
MainAssistant.prototype.SuccessOn = function(event) {
	mount=true;
	this.controller.get('StartButton').mojo.deactivate();
	this.controller.serviceRequest('palm://com.nizovn.androidchroot.shutdown.c', {
					method: "autoshutdown",
					parameters: {"state": "on"},
					onSuccess: this.SuccessOff.bind(this),
					onFailure: function(){}
				});
	this.StartClient();
};
MainAssistant.prototype.StartClient= function() {
	this.controller.serviceRequest('palm://com.nizovn.androidchroot.client.c', {
					method: "client",
					parameters: {"state": "on"},
					onSuccess: this.ExitClient.bind(this),
					onFailure: function(){},
				});
	client=true;
	this.buttonmodelstart.disabled = true;
	this.buttonmodelstop.disabled = false;
	this.controller.modelChanged(this.buttonmodelstart);
	this.controller.modelChanged(this.buttonmodelstop);
};
MainAssistant.prototype.ExitClient = function(event) {
	client=false;	
	mount=(event.mounted=="true")?true:false;
	this.Main();
};
MainAssistant.prototype.SuccessOff = function(event) {
	mount=false;
	client=false;
	this.Main();
};
var mount=false;
var client=false;
MainAssistant.prototype.activate = function(event){
	this.controller.serviceRequest('palm://com.nizovn.androidchroot.main.c', {
					method: "getstate",
					parameters: {"target": "files"},
					onSuccess: function(){},
					onFailure: this.FailureFiles.bind(this)
				});
	this.controller.serviceRequest('palm://com.nizovn.androidchroot.main.c', {
					method: "getstate",
					parameters: {"target": "swap"},
					onSuccess: function(){},
					onFailure: this.FailureSwap.bind(this)
				});
	this.controller.serviceRequest('palm://com.nizovn.androidchroot.main.c', {
					method: "getstate",
					parameters: {"target": "mount"},
					onSuccess: this.MountSuccess.bind(this),
					onFailure: this.MountFailure.bind(this)
				});
};
MainAssistant.prototype.FailureFiles = function(event) {
	this.controller.get('textFieldInfo').mojo.setValue("File "+event.file+" not found\n");
};
MainAssistant.prototype.FailureSwap = function(event) {
	var text=this.controller.get('textFieldInfo').mojo.getValue();
	this.controller.get('textFieldInfo').mojo.setValue(text+"WARNING: swap.ext3 not found\n");
};
MainAssistant.prototype.MountSuccess= function(event) {
	mount=true;
	this.AfterMount();
};
MainAssistant.prototype.MountFailure= function(event) {
	mount=false;
	this.AfterMount();
};
MainAssistant.prototype.ClientSuccess= function(event) {
	client=true;
	this.Main();
};
MainAssistant.prototype.ClientFailure= function(event) {
	client=false;
	this.Main();
};
MainAssistant.prototype.Main= function() {
	if((mount==true)&&(client==true)){
		this.buttonmodelstart.label= "Start client";
		this.buttonmodelstart.buttonClass= "affirmative";
		this.buttonmodelstart.disabled= true;
		this.buttonmodelstop.label= "Send shutdown";
		this.buttonmodelstop.buttonClass= "negative";
		this.buttonmodelstop.disabled= false;
	};
	if((mount==true)&&(client==false)){
		this.buttonmodelstart.label= "Start client";
		this.buttonmodelstart.buttonClass= "affirmative";
		this.buttonmodelstart.disabled= false;
		this.buttonmodelstop.label= "Send shutdown";
		this.buttonmodelstop.buttonClass= "negative";
		this.buttonmodelstop.disabled= true;
	};
	if((mount==false)&&(client==true)){};
	if((mount==false)&&(client==false)){
		this.buttonmodelstart.label= "Start android && Start client";
		this.buttonmodelstart.buttonClass= "affirmative";
		this.buttonmodelstart.disabled= false;
		this.buttonmodelstop.label= "Send shutdown";
		this.buttonmodelstop.buttonClass= "negative";
		this.buttonmodelstop.disabled= true;
	};
	this.controller.modelChanged(this.buttonmodelstart,this);
	this.controller.modelChanged(this.buttonmodelstop);
};
MainAssistant.prototype.AfterMount= function() {
	this.controller.serviceRequest('palm://com.nizovn.androidchroot.main.c', {
					method: "getstate",
					parameters: {"target": "client"},
					onSuccess: this.ClientSuccess.bind(this),
					onFailure: this.ClientFailure.bind(this),
				});
};
MainAssistant.prototype.deactivate = function(event) {};
MainAssistant.prototype.cleanup = function(event) {
	Mojo.Event.stopListening(this.controller.get("StartButton"),Mojo.Event.tap, this.TapStart.bind(this));
	Mojo.Event.stopListening(this.controller.get("StopButton"),Mojo.Event.tap, this.TapStop.bind(this));
};
