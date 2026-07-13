const { getDefaultConfig } = require('expo/config');
// Подключаем официальный метод создания списка исключений для Metro
const exclusionList = require('metro-config/src/defaults/exclusionList');

const config = getDefaultConfig(__dirname);

// Корректно исключаем папку modules из JS-бандлинга
config.resolver.blacklistRE = exclusionList([/modules\/.*/]);

module.exports = config;
