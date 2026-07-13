const { getDefaultConfig } = require('expo/config');

const config = getDefaultConfig(__dirname);

// Заставляем Metro полностью игнорировать папку modules при сборке JS
config.resolver.blacklistRE = /modules\/.*/;

module.exports = config;
