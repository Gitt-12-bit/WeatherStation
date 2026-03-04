from flask import Flask
from .config import Config
from .functions import DB

app = Flask(__name__)
mongo = DB(Config)

try:
    from .mqtt import Mqtt
except ImportError:
    Mqtt = None

from . import views