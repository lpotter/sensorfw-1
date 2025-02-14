/**
   @file config.cpp
   @brief Configuration handler

   <p>
   Copyright (C) 2009-2010 Nokia Corporation

   @author Matias Muhonen <ext-matias.muhonen@nokia.com>
   @author Lihan Guo <ext-lihan.guo@nokia.com>

   This file is part of Sensord.

   Sensord is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License
   version 2.1 as published by the Free Software Foundation.

   Sensord is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with Sensord.  If not, see <http://www.gnu.org/licenses/>.
   </p>
*/

#include "config.h"

#include "logging.h"
#include <QSettings>
#include <QVariant>
#include <QFile>
#include <QDir>
#include <QList>

static Config *static_configuration = 0;

Config::Config() {
}

Config::~Config() {
}

void Config::clearConfig() {
    m_settings.clear();
}

bool Config::loadConfig(const QString &defConfigPath, const QString &configDPath) {
    /* Not having config files is ok, failing to load one that exists is not */
    bool ret = true;
    if (!static_configuration) {
        static_configuration = new Config();
    }
    /* Process config.d dir in alnum order */
    if (!configDPath.isEmpty()) {
        QDir dir(configDPath, "*.conf", QDir::Name, QDir::Files);
        foreach(const QString &file, dir.entryList()) {
            if (!static_configuration->loadConfigFile(dir.absoluteFilePath(file))) {
                ret = false;
            }
        }
    }
    /* Primary config file overrides config.d */
    if (!defConfigPath.isEmpty() && QFile::exists(defConfigPath) ) {
        if (!static_configuration->loadConfigFile(defConfigPath))
            ret = false;
    }
    return ret;
}

bool Config::loadConfigFile(const QString &configFileName) {
    /* Success means the file was loaded and processed without hiccups */
    bool loaded = false;
    if (!QFile::exists(configFileName)) {
        sensordLogW() << "File does not exists \"" << configFileName <<  "\"";
    } else {
        QSettings merge(configFileName, QSettings::IniFormat);
        QSettings::Status status(merge.status());
        if (status == QSettings::FormatError ) {
            sensordLogW() << "Configuration file \"" << configFileName <<  "\" is in wrong format";
        } else if (status != QSettings::NoError) {
            sensordLogW() << "Unable to open \"" << configFileName <<  "\" configuration file";
        } else {
            foreach (const QString &key, merge.allKeys()) {
                m_settings.setValue(key, merge.value(key));
            }
            loaded = true;
        }
    }
    return loaded;
}

QVariant Config::value(const QString &key) const {
    QVariant var = m_settings.value(key, QVariant());
    if(var.isValid()) {
        sensordLogT() << "Value for key" << key << ":" << var.toString();
    }
    return var;
}

QStringList Config::groups() const
{
    QStringList groups = m_settings.childGroups();
    return groups;
}

Config *Config::configuration() {
    if (!static_configuration) {
        sensordLogW() << "Configuration has not been loaded";
    }
    return static_configuration;
}

void Config::close() {
    delete static_configuration;
    static_configuration = 0;
}

bool Config::exists(const QString &key) const
{
    return value(key).isValid();
}
