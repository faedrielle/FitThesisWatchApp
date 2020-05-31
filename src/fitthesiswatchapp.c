#include "fitthesiswatchapp.h"
#include <sensor.h>
#include <bluetooth.h>
#include <dlog.h>

//UUID
const char* server_uuid="00000000-0000-04d2-0000-00000000162e";
char *bt_server_address = "A8:9C:ED:40:7E:C2";
const char *remote_server_name = "server device";
bt_device_info_s *server_info = NULL;
//default socket
int server_socket_fd = -1;

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label;
} appdata_s;

void
socket_connection_state_changed(int result, bt_socket_connection_state_e connection_state,
                                bt_socket_connection_s *connection, void *user_data);
void
adapter_state_changed_cb(int result, bt_adapter_state_e adapter_state, void* user_data);
bool
adapter_bonded_device_cb(bt_device_info_s *device_info, void *user_data);

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}

static void
create_base_gui(appdata_s *ad)
{
	/* Window */
	/* Create and initialize elm_win.
	   elm_win is mandatory to manipulate window. */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Conformant */
	/* Create and initialize elm_conformant.
	   elm_conformant is mandatory for base gui to have proper size
	   when indicator or virtual keypad is visible. */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Label */
	/* Create an actual view of the base gui.
	   Modify this part to change the view. */
	ad->label = elm_label_add(ad->conform);
	elm_object_text_set(ad->label, "<align=center>Hello Tizen</align>");
	evas_object_size_hint_weight_set(ad->label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(ad->conform, ad->label);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);

	/*initialize BT*/
	int ret = bt_initialize();
	if (ret != BT_ERROR_NONE) {
	    dlog_print(DLOG_ERROR, LOG_TAG, "MYLOG: [bt_initialize] failed.");

	    return;
	}

	ret = bt_adapter_set_state_changed_cb(adapter_state_changed_cb, NULL);
	if (ret != BT_ERROR_NONE)
	    dlog_print(DLOG_ERROR, LOG_TAG, "MYLOG: [bt_adapter_set_state_changed_cb()] failed.");

	ret = bt_adapter_foreach_bonded_device(adapter_bonded_device_cb, remote_server_name);
		if (ret != BT_ERROR_NONE) {
		    dlog_print(DLOG_ERROR, LOG_TAG, "MYLOG: [bt_adapter_foreach_bonded_device] failed!");
		    return;
	}
	ret = bt_socket_set_connection_state_changed_cb(socket_connection_state_changed, NULL);
	if (ret != BT_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "MYLOG: [bt_socket_set_connection_state_changed_cb] failed.");

		return;
	}
	ret = bt_socket_connect_rfcomm(bt_server_address, server_uuid);
			if (ret != BT_ERROR_NONE) {
			    dlog_print(DLOG_ERROR, LOG_TAG, "MYLOG: [bt_socket_connect_rfcomm] failed.");
			    return;
			} else {
			    dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: [bt_socket_connect_rfcomm] Succeeded. bt_socket_connection_state_changed_cb will be called.");
	}
}

static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = data;

	create_base_gui(ad);

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{


}

static void
app_terminate(void *data)
{
    bt_adapter_unset_state_changed_cb();
	bt_socket_unset_connection_state_changed_cb();
	int ret = bt_socket_disconnect_rfcomm(server_socket_fd);
	if (ret != BT_ERROR_NONE)
	    dlog_print(DLOG_ERROR, LOG_TAG, "MYLOG: [bt_socket_destroy_rfcomm] failed.");

	bt_deinitialize();
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

bool isHRSensorSupported() {
	sensor_type_e type = SENSOR_HRM;


	bool supported;
	int error = sensor_is_supported(type, &supported);
	if (error != SENSOR_ERROR_NONE) {
	    dlog_print(DLOG_ERROR, LOG_TAG, "sensor_is_supported error: %d", error);
	    return false;
	}

	if(supported){
	    dlog_print(DLOG_DEBUG, LOG_TAG, "HRM is%s supported", supported ? "" : " not");
//	    sprintf(out,"HRM is%s supported", supported ? "" : " not");
//	    elm_object_text_set(event_label, out);
	}
	return true;

}

void on_sensor_event(sensor_h sensor, sensor_event_s *event, void *user_data){
    // Select a specific sensor with a sensor handle
    sensor_type_e type;
    sensor_get_type(sensor, &type);

    switch (type) {
    case SENSOR_HRM:
    	dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: %d" , event->values[0]);
    	char a[100];
    	sprintf(a,"%f", event->values[0]);
//    	elm_object_text_set(event_label, a);
    	break;
    default:
        dlog_print(DLOG_ERROR, LOG_TAG, "MYLOG: Not an HRM event");
    }
}

void
adapter_state_changed_cb(int result, bt_adapter_state_e adapter_state, void* user_data)
{
    if (result != BT_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "MYLOG: [adapter_state_changed_cb] failed! result=%d", result);

        return;
    }
    if (adapter_state == BT_ADAPTER_ENABLED) {
        dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: [adapter_state_changed_cb] Bluetooth is enabled!");

        /* Get information about Bluetooth adapter */
        char *local_address = NULL;
        bt_adapter_get_address(&local_address);
        dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: [adapter_state_changed_cb] Adapter address: %s.", local_address);
        if (local_address)
            free(local_address);
        char *local_name;
        bt_adapter_get_name(&local_name);
        dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: [adapter_state_changed_cb] Adapter name: %s.", local_name);
        if (local_name)
            free(local_name);
        /* Visibility mode of the Bluetooth device */
        bt_adapter_visibility_mode_e mode;
        /*
           Duration until the visibility mode is changed
           so that other devices cannot find your device
        */
        int duration = 1;
        bt_adapter_get_visibility(&mode, &duration);
        switch (mode) {
        case BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE:
            dlog_print(DLOG_INFO, LOG_TAG,
                       "MYLOG: [adapter_state_changed_cb] Visibility: NON_DISCOVERABLE");
            break;
        case BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE:
            dlog_print(DLOG_INFO, LOG_TAG,
                       "MYLOG: [adapter_state_changed_cb] Visibility: GENERAL_DISCOVERABLE");
            break;
        case BT_ADAPTER_VISIBILITY_MODE_LIMITED_DISCOVERABLE:
            dlog_print(DLOG_INFO, LOG_TAG,
                       "MYLOG: [adapter_state_changed_cb] Visibility: LIMITED_DISCOVERABLE");
            break;
        }
    } else {
        dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: [adapter_state_changed_cb] Bluetooth is disabled!");
        /*
           When you try to get device information
           by invoking bt_adapter_get_name(), bt_adapter_get_address(),
           or bt_adapter_get_visibility(), BT_ERROR_NOT_ENABLED occurs
        */
    }
}

bool
adapter_bonded_device_cb(bt_device_info_s *device_info, void *user_data)
{
    if (device_info == NULL)
        return true;
    /* Get information about bonded device */
    dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: Get information about the bonded device");
    dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: remote address = %s.", device_info->remote_address);
    dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: remote name = %s.", device_info->remote_name);
    dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: bonded?? %d.", device_info->is_bonded);
    dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: connected?? %d.", device_info->is_connected);
    dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: authorized?? %d.", device_info->is_authorized);
    if (!strcmp(device_info->remote_address, bt_server_address)) {
        dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: The server device is found in bonded device list. address(%s)",
                   device_info->remote_address);
        remote_server_name = strdup(device_info->remote_name);
        server_info = device_info;
        return false;
    }
    /* Keep iterating */

    return true;
}

void
socket_connection_state_changed(int result, bt_socket_connection_state_e connection_state,
                                bt_socket_connection_s *connection, void *user_data)
{
	dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: this is socket_connection_state_changed");
    if (result != BT_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "MYLOG: [socket_connection_state_changed_cb] failed. result =%d.", result);

        return;
    }

    if (connection_state == BT_SOCKET_CONNECTED) {
        dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: Callback: Connected.");
        if (connection != NULL) {
            dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: Callback: Socket of connection - %d.", connection->socket_fd);
            dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: Callback: Role of connection - %d.", connection->local_role);
            dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: Callback: Address of connection - %s.", connection->remote_address);
            /* socket_fd is used for sending data and disconnecting a device */
            server_socket_fd = connection->socket_fd;

            char data[] = "Sending test";


			dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: [bt_socket_send_data]");
			int ret = bt_socket_send_data(server_socket_fd, data, sizeof(data));
			if (ret != BT_ERROR_NONE)
				dlog_print(DLOG_ERROR, LOG_TAG, "MYLOG: [bt_socket_send_data] failed, error: %d", ret);



        } else {
            dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: Callback: No connection data");
        }
    } else {
        dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: Callback: Disconnected.");
        if (connection != NULL) {
            dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: Callback: Socket of disconnection - %d.", connection->socket_fd);
            dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: Callback: Address of connection - %s.", connection->remote_address);
        } else {
            dlog_print(DLOG_INFO, LOG_TAG, "MYLOG: Callback: No connection data");
        }
    }
}


int
main(int argc, char *argv[])
{
//	event_listener event_listener;
	sensor_h sensor;

	//BEGIN some code from template
	appdata_s ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}
	//END some code from template


	if (!isHRSensorSupported()) {
		return 1;
	}




	return 0;
}
