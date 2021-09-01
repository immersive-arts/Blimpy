autowatch = 1;

outlets = 3;
setinletassist(0,"settings");


var OUTID_CONTROL = 0;
var OUTID_SETUP = 1;
var OUTID_TOPIC_ROUTE = 1;
var OUTID_PUBLISH = 2;

setoutletassist(OUTID_CONTROL,"control");
setoutletassist(OUTID_SETUP,"setup");
setoutletassist(OUTID_PUBLISH,"publish/subscribe");
// API

// manager topics
var API_add 		= "/add";
var API_remove 		= "/remove";
var API_heartbeat 	= "/heartbeat"; // heartbeat command sent from the manager

// managed device topics
var API_feedback 	= "/feedback"; 	// feedback sent from the manager
var API_state 		= "/state"; 	// state command sent from the manager
var API_clear 		= "/clear"; 	// clear command sent to the manager
var API_config      = "/config";    // config command sent to the manager
var API_stack 		= "/stack"; 	// stack command sent to the manager
var API_stack_ready 		= "ready";
var API_stack_hold 			= "hold";
var API_stack_freeze 		= "freeze";
var API_stack_park 			= "park";
var API_stack_move 			= "move";
var API_stack_untracked 	= "untracked";
var API_stack_unmanaged 	= "unmanaged";
var API_stack_unavailable 	= "unavailable";


// device setup
var my_manager_base_topic = null;
var my_device_base_topic = null;
var my_device_name = null;
var my_device_type = null;
var my_tracking_id = -1;
var my_update_fps = 0; 
var my_data_fps = 0;
var my_velocity_max = 0.0;

// device config
var my_k_p_z = 0.0;
var my_k_d_z = 0.0;
var my_k_i_z = 0.0;
var my_k_p_xy = 0.0;
var my_k_d_xy = 0.0;
var my_k_p_a = 0.0;
var my_k_d_a = 0.0;
var my_config_payload = "";

var my_managedDevAddress = null;
var my_devAddress = null;
var my_deviceState = null;

var my_topic_AddDevice 			= null;
var my_topic_RemoveDevice 		= null;
var my_topic_ManagerHeartbeat 	= null;

var my_topic_DeviceFeedback 	= null;
var my_topic_DeviceState 		= null;
var my_topic_DeviceStackClear 	= null;
var my_topic_DeviceStack 		= null;
var my_topic_DeviceConfig 		= null;

// device condition

var myState     = "undefined";
var myEnable 	= false; // this module is enabled
var myConnected = false; // connected to the network
var myHandshaked= false; // receive handshake from manager
var myManaged 	= false; // managed by the manager

// FLAGS
var flg_addressChange = false;
var flg_guiChange = false;
var flg_waitingForReady = false;

if (jsarguments.length>1)
	myval = jsarguments[1];


function done(){
	update();
}

// sent by the mqtt client
function connected(_connected){
    if(myConnected !== (_connected == 1)?true:false){
        myConnected = (_connected == 1)?true:false;
        if(!myConnected){
            myHandshaked = false;
            myManaged = false;
            flg_guiChange = true;
        }
        update();
    }
}

// sent by the manager
function state(_state){
    //post("state: " + _state + " \n");
    if(myState !== _state){
        if(_state === API_stack_untracked){
            myState = API_stack_untracked;
        } else {
            // if we receive these messages, then
            myManaged = true;

            if(_state === API_stack_ready){
                if(flg_waitingForReady){
                    publish(my_topic_DeviceConfig, my_config_payload);
                    flg_waitingForReady = false;
                }
                myState = API_stack_ready;
            }
            else if(_state === API_stack_hold){
                myState = API_stack_hold;
            }
            else if(_state === API_stack_freeze){
                myState = API_stack_freeze;
            }
            else if(_state === API_stack_park){
                myState = API_stack_park;
            }
            else if(_state === API_stack_move){
                myState = API_stack_move;
            }        
        }
        flg_guiChange = true;
        update();        
    }
}

function command(_cmd, _device){
	if(_device == my_device_name || _device == undefined || _device == "bang"){
		if(_cmd == API_stack_freeze){
            outlet(OUTID_CONTROL, API_stack_freeze);
		}
		if(_cmd == API_stack_park){
            outlet(OUTID_CONTROL, API_stack_park);
		}
		if(_cmd == "hot"){
            outlet(OUTID_CONTROL, "hot");
		}
		if(_cmd == "addDevice"){
            addDevice(_device);
		}
		if(_cmd == "removeDevice"){
            removeDevice(_device);
            outlet(OUTID_CONTROL, API_stack_unmanaged);
		}
	}
}

// sent by the manager
function heartbeat(_heartbeat){
    if(myHandshaked !== (_heartbeat == 1)?true:false){
        myHandshaked = (_heartbeat == 1)?true:false;
        myConnected = myHandshaked;
        flg_guiChange = true;
        update();
    }
}

function addDevice(_deviceName){
	if(myEnable){
        if(myConnected){            
            if(myHandshaked){		
                if(_deviceName === my_device_name || _deviceName === undefined || _device == "bang"){		
                    var payload = "device_base_topic="+ my_device_base_topic + " device_name=" + my_device_name + " device_type=" + my_device_type + " tracking_id=" + my_tracking_id;
                    publish(my_topic_AddDevice, payload);
                    myManaged = true;
                    flg_guiChange = true;
                    flg_waitingForReady = true;
                    update();
                }
            } else {
                error("adding device '" + my_device_name + "' failed. No connection to manager available\n");
            }
        } else {
            error("adding device '" + my_device_name + "' failed. No connection to mqtt broker available\n");            
        }
    } else {
        error("adding device '" + my_device_name + "' failed. Device not enabled\n");            
    }
}

function removeDevice(_deviceName){
	if(myEnable){
        if(myConnected){            
            if(myHandshaked){		
                if(_deviceName === my_device_name || _deviceName === undefined || _device == "bang"){		
                    var payload = "device_name=" + my_device_name;
                    publish(my_topic_RemoveDevice, payload);
                    myManaged = false;
                    flg_guiChange = true;
					myState = API_stack_unmanaged;
                    update();
                }
            } else {
                error("removing device '" + my_device_name + "' failed. No connection to manager available\n");
            }
        } else {
            error("removing device '" + my_device_name + "' failed. No connection to mqtt broker available\n");            
        }
    } else {
        error("removing device '" + my_device_name + "' failed. Device not enabled\n");            
    }
}

function msg_int(v)
{
    enable(v);
}

function enable(_enabled){
	myEnable = (_enabled == 1)?true:false;
    if(!myEnable){
        unsubscribe(my_topic_DeviceState);
        unsubscribe(my_topic_DeviceFeedback);
        unsubscribe(my_topic_ManagerHeartbeat);

        myHandshaked = false;
        myManaged = false;
        flg_guiChange = true;
    } else {
        subscribe(my_topic_ManagerHeartbeat);
        subscribe(my_topic_DeviceState);
        subscribe(my_topic_DeviceFeedback);

    }
	update();
}

function update(_enforce)
{
	if(flg_addressChange || _enforce == true){
        my_devAddress =  my_device_base_topic + "/" + my_device_name;
		my_managedDevAddress = my_manager_base_topic + "/" + my_devAddress;
		
		my_topic_AddDevice 			= my_manager_base_topic + API_add;
		my_topic_RemoveDevice 		= my_manager_base_topic + API_remove;
		my_topic_ManagerHeartbeat 	= my_manager_base_topic + API_heartbeat;

		my_topic_DeviceFeedback 	= my_managedDevAddress + API_feedback;
		my_topic_DeviceState 		= my_managedDevAddress + API_state;
		my_topic_DeviceStackClear 	= my_managedDevAddress + API_clear;
		my_topic_DeviceStack 		= my_managedDevAddress + API_stack;
        my_topic_DeviceConfig       = my_managedDevAddress + API_config;
                
        routeTopic("heartbeat", my_topic_ManagerHeartbeat);
        routeTopic("state", my_topic_DeviceState);
        routeTopic("feedback", my_topic_DeviceFeedback);

		outlet(OUTID_SETUP, "stack", "set", my_topic_DeviceStack);
		outlet(OUTID_SETUP, "clear", "set", my_topic_DeviceStackClear);
		outlet(OUTID_SETUP, "display", "set", my_managedDevAddress);
        
		flg_addressChange = false;
	}
    
    if(flg_guiChange || _enforce == true){
        outlet(OUTID_SETUP, "state", myState);
        outlet(OUTID_SETUP, "gui", "managed", myManaged);
        outlet(OUTID_SETUP, "gui", "connected", myConnected);
        outlet(OUTID_SETUP, "gui", "heartbeat", myHandshaked);
        outlet(OUTID_SETUP, "gui", "config", "hidden", myManaged);
        outlet(OUTID_SETUP, "gui", "menu_add", "hidden", !myHandshaked);       
        
        flg_guiChange = false;
    }
}

function routeTopic(_cmd, _topic){
    outlet(OUTID_TOPIC_ROUTE, "route", _cmd, _topic);
}

function publish(_topic, _payload){
	if(_topic !== null)
		outlet(OUTID_PUBLISH, "publish", _topic, _payload);
}

function subscribe(_topic){
	if(_topic !== null)
		outlet(OUTID_PUBLISH, "subscribe", _topic);
}

function unsubscribe(_topic){
	if(_topic !== null)
		outlet(OUTID_PUBLISH, "unsubscribe", _topic);
}

// SET FUNCTIONS

function device_base_topic(_str){
	my_device_base_topic = _str;
	flg_addressChange = true;
}

function manager_base_topic(_str){
	my_manager_base_topic = _str;
	flg_addressChange = true;
}

function device_type(_str){
	my_device_type = _str;
	flg_addressChange = true;
}

function device_name(_str){
	my_device_name = _str;
	flg_addressChange = true;
}

function tracking_id(_int){
	my_tracking_id = _int;
	flg_addressChange = true;
}

function data_fps(_int){
	my_data_fps = _int;
}

function update_fps(_int){
	my_update_fps = _int;
	outlet(OUTID_SETUP, "update_ms", 1000.0 / my_update_fps);
}

function velocity_max(_float){
	my_velocity_max = _float;
}

function calc_config(){
	var a = arrayfromargs(arguments);
	my_k_p_z = a[0];
	my_k_d_z = a[1];
	my_k_i_z = a[2];
	my_k_p_xy = a[3];
	my_k_d_xy = a[4];
	my_k_p_a = a[5];
	my_k_d_a = a[6];
    my_config_payload = "k_p_z=" + my_k_p_z + " k_d_z=" + my_k_d_z + " k_i_z=" + my_k_i_z + " k_p_xy=" + my_k_p_xy + " k_d_xy=" + my_k_d_xy + " k_p_a=" + my_k_p_a + " k_d_a=" + my_k_d_a;
    publish(my_topic_DeviceConfig, my_config_payload);
}


// device config


function anything()
{
	var a = arrayfromargs(messagename, arguments);
	post("received message " + a + "\n");
}

// error("What just happened?\n");
// post(1,2,3,"violet",a);
// cpost (any arguments); -> to system console:
// messnamed (Max object name, message name, any arguments)
// messnamed("flower","bang”);
// a = new Array(900,1000,1100);
// jsthis -> this instance
//  autowatch (1) -> global code -> autoload after js-file was edited and saved
