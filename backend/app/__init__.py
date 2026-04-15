from flask import Flask
from .config import Config
from .functions import DB
from flask_cors import CORS



app = Flask(__name__)
mongo = DB(Config)
CORS(app)

try:
    from .mqtt import Mqtt
except ImportError:
    Mqtt = None

from . import views