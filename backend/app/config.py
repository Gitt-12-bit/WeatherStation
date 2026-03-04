import os
from os import  environ
from os.path import  abspath, dirname 
from dotenv import load_dotenv

load_dotenv()  # load environment variables from .env if it exists.
basedir = abspath(dirname(__file__))

class Config(object):
    """Base Config Object"""
    FLASK_DEBUG = os.getenv("FLASK_DEBUG", "False") == "True"
    
    # Use safe defaults if env vars are missing
    FLASK_RUN_HOST = os.getenv("FLASK_RUN_HOST", "127.0.0.1")
    FLASK_RUN_PORT = int(os.getenv("FLASK_RUN_PORT", 5000))

    # Your MongoDB/other configs
    DB_USERNAME = os.getenv("DB_USERNAME", "user")
    DB_PASSWORD = os.getenv("DB_PASSWORD", "pass")
    DB_NAME = os.getenv("DB_NAME", "homeautomation")
    
    
    MQTT_HOST = os.getenv("MQTT_HOST", "127.0.0.1")
    MQTT_PORT = int(os.getenv("MQTT_PORT", 1883))
    MQTT_USERNAME = os.getenv("MQTT_USERNAME", "")
    MQTT_PASSWORD = os.getenv("MQTT_PASSWORD", "")
    

    FLASK_DEBUG                             = eval(environ.get('DEBUG','False'))
    SECRET_KEY                              = environ.get('SECRET_KEY', 'Som3$ec5etK*y')
    UPLOADS_FOLDER                          = environ.get('UPLOADS_FOLDER') 
    IMAGE_FOLDER                            = environ.get('IMAGE_FOLDER') 

    ENV                                     = environ.get('FLASK_DEBUG') 
    FLASK_RUN_PORT                          = environ.get('FLASK_RUN_PORT') 
    FLASK_RUN_HOST                          = environ.get('FLASK_RUN_HOST') 

    # MONGODB VARIABLES
    DB_USERNAME                             = environ.get('DB_USERNAME') 
    DB_PASSWORD                             = environ.get('DB_PASSWORD') 
    DB_SERVER                               = environ.get('DB_SERVER') 
    DB_PORT                                 = environ.get('DB_PORT') 

    PROPAGATE_EXCEPTIONS                    = False
 
 
