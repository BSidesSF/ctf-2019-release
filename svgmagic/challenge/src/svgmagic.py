import flask
import cairosvg
import base64
import urllib.request
from defusedxml import ElementTree


app = flask.Flask(__name__)
app.config['MAX_CONTENT_LENGTH'] = 1 * 1024 * 1024


def defusedefused():
    def external_handler(parser, context, base, sysid, pubid):
        print('parser: {}\ncontext: {}\nbase: {}\nsysid: {}\npubid: {}\n'.format(
            parser, context, base, sysid, pubid))
        data = urllib.request.urlopen(sysid).read()
        child = parser.parser.ExternalEntityParserCreate(context)
        child.Parse(data)
        return 1

    d = ElementTree.DefusedXMLParser
    d.defused_entity_decl = None
    d.defused_unparsed_entity_decl = None
    d.defused_external_entity_ref_handler = external_handler


@app.route("/")
def home():
    return flask.render_template("index.html")


@app.route("/render", methods=["POST"])
def render():
    if 'svgfile' not in flask.request.files:
        flask.abort(400)
    fp = flask.request.files['svgfile']
    if not fp.filename:
        flask.abort(400)
    svgdata = fp.read()
    try:
        pngdata = cairosvg.svg2png(bytestring=svgdata)
    except Exception as ex:
        app.logger.exception("Error parsing svg: %s", ex)
        flask.abort(500)
    svguri = "data:image/svg+xml;base64," + base64.b64encode(svgdata).decode('utf-8')
    pnguri = "data:image/png;base64," + base64.b64encode(pngdata).decode('utf-8')
    return flask.render_template("render.html", svguri=svguri, pnguri=pnguri)


defusedefused()
