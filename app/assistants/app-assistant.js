// stage names
var mainStageName = 'androidchroot-main';

function AppAssistant() {};

AppAssistant.prototype.handleLaunch = function(params)
{
	var mainStageController = this.controller.getStageController(mainStageName);

	try {
		if (!params) {
			console.log("handleLaunch: no params");
			if (mainStageController) {
				mainStageController.popScenesTo('main');
				mainStageController.activate();
			}
			else {
				this.controller.createStageWithCallback({name: mainStageName, lightweight: true},
									this.launchFirstScene.bind(this));
			}
		}
		else {
			console.log("handleLaunch: "+JSON.stringify(params));
			if (mainStageController) {
				mainStageController.popScenesTo('main', params);
				mainStageController.activate();
			}
			else {
				this.controller.createStageWithCallback({name: mainStageName, lightweight: true},
									this.launchFirstScene.bind(this, params));
			}
		}
	}
	catch (e) {
		Mojo.Log.logException(e, "AppAssistant#handleLaunch");
	}	
};

AppAssistant.prototype.launchFirstScene = function(controller, params)
{
	console.log("launchFirstScene: "+JSON.stringify(params));
	controller.pushScene('main', params);
};
