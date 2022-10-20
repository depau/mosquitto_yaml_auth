#include "llvm/Support/Error.h"

#include "AuthManager.h"

extern "C" {
#include <mosquitto.h>
#include <mosquitto_broker.h>
#include <mosquitto_plugin.h>
}

#if !defined(LIBMOSQUITTO_VERSION_NUMBER) || LIBMOSQUITTO_VERSION_NUMBER < 1005001
#error "mosquitto 1.5.1 or higher is required"
#endif

extern "C" {
/*
 * Function: mosquitto_auth_plugin_version
 *
 * The broker will call this function immediately after loading the plugin to
 * check it is a supported plugin version. Your code must simply return
 * the version of the plugin interface you support, i.e. 4.
 */
int mosquitto_auth_plugin_version(void) {
  return 4;
}

/*
 * Function: mosquitto_auth_plugin_init
 *
 * Called after the plugin has been loaded and <mosquitto_auth_plugin_version>
 * has been called. This will only ever be called once and can be used to
 * initialise the plugin.
 *
 * Parameters:
 *
 *	user_data -      The pointer set here will be passed to the other plugin
 *	                 functions. Use to hold connection information for example.
 *	opts -           Pointer to an array of struct mosquitto_opt, which
 *	                 provides the plugin options defined in the configuration file.
 *	opt_count -      The number of elements in the opts array.
 *
 * Return value:
 *	Return 0 on success
 *	Return >0 on failure.
 */
int mosquitto_auth_plugin_init(void **user_data, struct mosquitto_opt *opts, int opt_count) {
  *user_data = new AuthManager();
  return 0;
}

/*
 * Function: mosquitto_auth_plugin_cleanup
 *
 * Called when the broker is shutting down. This will only ever be called once
 * per plugin.
 * Note that <mosquitto_auth_security_cleanup> will be called directly before
 * this function.
 *
 * Parameters:
 *
 *	user_data -      The pointer provided in <mosquitto_auth_plugin_init>.
 *	opts -           Pointer to an array of struct mosquitto_opt, which
 *	                 provides the plugin options defined in the configuration file.
 *	opt_count -      The number of elements in the opts array.
 *
 * Return value:
 *	Return 0 on success
 *	Return >0 on failure.
 */
int mosquitto_auth_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count) {
  auto *authManager = static_cast<AuthManager *>(user_data);
  delete authManager;
  return 0;
}

/*
 * Function: mosquitto_auth_security_init
 *
 * This function is called in two scenarios:
 *
 * 1. When the broker starts up.
 * 2. If the broker is requested to reload its configuration whilst running. In
 *    this case, <mosquitto_auth_security_cleanup> will be called first, then
 *    this function will be called.  In this situation, the reload parameter
 *    will be true.
 *
 * Parameters:
 *
 *	user_data -      The pointer provided in <mosquitto_auth_plugin_init>.
 *	opts -           Pointer to an array of struct mosquitto_opt, which
 *	                 provides the plugin options defined in the configuration file.
 *	opt_count -      The number of elements in the opts array.
 *	reload -         If set to false, this is the first time the function has
 *	                 been called. If true, the broker has received a signal
 *	                 asking to reload its configuration.
 *
 * Return value:
 *	Return 0 on success
 *	Return >0 on failure.
 */
int mosquitto_auth_security_init(void *user_data, struct mosquitto_opt *opts, int opt_count, bool reload) {
  auto *authManager = static_cast<AuthManager *>(user_data);
  llvm::Error maybeErr = authManager->loadConfig(opts, opt_count);

  if (maybeErr) {
    llvm::errs() << "failed to load config: " << maybeErr << '\n';
    return 1;
  }
  return 0;
}

/*
 * Function: mosquitto_auth_security_cleanup
 *
 * This function is called in two scenarios:
 *
 * 1. When the broker is shutting down.
 * 2. If the broker is requested to reload its configuration whilst running. In
 *    this case, this function will be called, followed by
 *    <mosquitto_auth_security_init>. In this situation, the reload parameter
 *    will be true.
 *
 * Parameters:
 *
 *	user_data -      The pointer provided in <mosquitto_auth_plugin_init>.
 *	opts -           Pointer to an array of struct mosquitto_opt, which
 *	                 provides the plugin options defined in the configuration file.
 *	opt_count -      The number of elements in the opts array.
 *	reload -         If set to false, this is the first time the function has
 *	                 been called. If true, the broker has received a signal
 *	                 asking to reload its configuration.
 *
 * Return value:
 *	Return 0 on success
 *	Return >0 on failure.
 */
int mosquitto_auth_security_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count, bool reload) {
  auto *authManager = static_cast<AuthManager *>(user_data);
  authManager->clearConfig();
  return 0;
}

/*
 * Function: mosquitto_auth_acl_check
 *
 * Called by the broker when topic access must be checked. access will be one
 * of:
 *  MOSQ_ACL_SUBSCRIBE when a client is asking to subscribe to a topic string.
 *                     This differs from MOSQ_ACL_READ in that it allows you to
 *                     deny access to topic strings rather than by pattern. For
 *                     example, you may use MOSQ_ACL_SUBSCRIBE to deny
 *                     subscriptions to '#', but allow all topics in
 *                     MOSQ_ACL_READ. This allows clients to subscribe to any
 *                     topic they want, but not discover what topics are in use
 *                     on the server.
 *  MOSQ_ACL_READ      when a message is about to be sent to a client (i.e. whether
 *                     it can read that topic or not).
 *  MOSQ_ACL_WRITE     when a message has been received from a client (i.e. whether
 *                     it can write to that topic or not).
 *
 * Return:
 *	MOSQ_ERR_SUCCESS if access was granted.
 *	MOSQ_ERR_ACL_DENIED if access was not granted.
 *	MOSQ_ERR_UNKNOWN for an application specific error.
 *	MOSQ_ERR_PLUGIN_DEFER if your plugin does not wish to handle this check.
 */
int mosquitto_auth_acl_check(void *user_data,
                             int access,
                             struct mosquitto *client,
                             const struct mosquitto_acl_msg *msg) {
  return MOSQ_ERR_PLUGIN_DEFER;
}

/*
 * Function: mosquitto_auth_unpwd_check
 *
 * This function is OPTIONAL. Only include this function in your plugin if you
 * are making basic username/password checks.
 *
 * Called by the broker when a username/password must be checked.
 *
 * Return:
 *	MOSQ_ERR_SUCCESS if the user is authenticated.
 *	MOSQ_ERR_AUTH if authentication failed.
 *	MOSQ_ERR_UNKNOWN for an application specific error.
 *	MOSQ_ERR_PLUGIN_DEFER if your plugin does not wish to handle this check.
 */
int mosquitto_auth_unpwd_check(void *user_data, struct mosquitto *client, const char *username, const char *password) {
  auto *authManager = static_cast<AuthManager *>(user_data);
  return authManager->checkUser(username, password) ? MOSQ_ERR_SUCCESS : MOSQ_ERR_AUTH;
}

/*
 * Function: mosquitto_psk_key_get
 *
 * This function is OPTIONAL. Only include this function in your plugin if you
 * are making TLS-PSK checks.
 *
 * Called by the broker when a client connects to a listener using TLS/PSK.
 * This is used to retrieve the pre-shared-key associated with a client
 * identity.
 *
 * Examine hint and identity to determine the required PSK (which must be a
 * hexadecimal string with no leading "0x") and copy this string into key.
 *
 * Parameters:
 *	user_data -   the pointer provided in <mosquitto_auth_plugin_init>.
 *	hint -        the psk_hint for the listener the client is connecting to.
 *	identity -    the identity string provided by the client
 *	key -         a string where the hex PSK should be copied
 *	max_key_len - the size of key
 *
 * Return value:
 *	Return 0 on success.
 *	Return >0 on failure.
 *	Return MOSQ_ERR_PLUGIN_DEFER if your plugin does not wish to handle this check.
 */
int mosquitto_auth_psk_key_get(void *user_data,
                               struct mosquitto *client,
                               const char *hint,
                               const char *identity,
                               char *key,
                               int max_key_len) {
  return MOSQ_ERR_PLUGIN_DEFER;
}
}