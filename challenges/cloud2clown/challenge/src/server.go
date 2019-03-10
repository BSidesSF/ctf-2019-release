package main

import (
	"encoding/json"
	"golang.org/x/net/html"
	"io"
	"io/ioutil"
	"log"
	"net"
	"net/http"
	"net/url"
	"os"
	"path"
	"strings"
	"time"
)

const reviewQueueSize = 1024

var reviewQueue chan string
var authzKey string
var port string

var replacements = [][2]string{
	{"cloud", "clown"},
	{"Cloud", "Clown"},
	{"CLOUD", "CLOWN"},
}

// SSRF flag
var SSRFFlag string

func getHTTPClient() *http.Client {
	return &http.Client{
		Timeout: time.Second * 5,
	}
}

func getAuthzKey() string {
	if authzKey != "" {
		return authzKey
	}
	authzKey = os.Getenv("AUTHZ_KEY")
	if authzKey == "" {
		authzKey = "correct horse battery staple"
	}
	return authzKey
}

func validateUrl(u string) bool {
	if u == "" {
		return false
	}
	if p, err := url.Parse(u); err != nil {
		return false
	} else {
		if p.Scheme != "http" && p.Scheme != "https" {
			return false
		}
		if p.Host == "localhost" || p.Host == "metadata.google.internal" || p.Host == "169.254.169.254" {
			return false
		}
	}
	return true
}

func retrieveAndRewrite(w http.ResponseWriter, u string) {
	client := getHTTPClient()
	log.Printf("Requesting url %s", u)
	if resp, err := client.Get(u); err != nil {
		log.Printf("Error loading: %s", err)
		http.Error(w, "Unable to retrieve page.", http.StatusBadGateway)
		return
	} else {
		rewritePage(resp.Body, w, u)
	}
}

func handleRender(w http.ResponseWriter, r *http.Request) {
	urlString := r.PostFormValue("url")
	if !validateUrl(urlString) {
		http.Error(w, "Invalid URL provided.", http.StatusBadRequest)
		return
	}
	w.Header().Set("X-Backend-Port", port)
	retrieveAndRewrite(w, urlString)
}

func rewritePage(r io.Reader, w io.Writer, baseUrlString string) error {
	baseUrl, err := url.Parse(baseUrlString)
	if err != nil {
		return err
	}

	rewriteAttr := func(n *html.Node, aName string) {
		for i, a := range n.Attr {
			if strings.ToLower(a.Key) == aName {
				if targetUrl, err := url.Parse(a.Val); err == nil {
					fixed := baseUrl.ResolveReference(targetUrl)
					log.Printf("%s -> %s", targetUrl.String(), fixed.String())
					// avoid copy by value issue
					n.Attr[i].Val = fixed.String()
				}
				break
			}
		}
	}

	injectButterBar := func(n *html.Node) {
		bbNode := &html.Node{
			Type: html.ElementNode,
			Data: "script",
			Attr: []html.Attribute{
				{
					Key: "src",
					Val: "/static/butterbar.js",
				},
				{
					Key: "data-base-url",
					Val: baseUrl.String(),
				},
			},
		}
		n.AppendChild(bbNode)
	}

	var rewriteNode func(n *html.Node)
	rewriteNode = func(n *html.Node) {
		switch n.Type {
		case html.TextNode:
			for _, replacePair := range replacements {
				n.Data = strings.Replace(n.Data, replacePair[0], replacePair[1], -1)
			}
		case html.ElementNode:
			switch strings.ToLower(n.Data) {
			case "script", "img":
				rewriteAttr(n, "src")
			case "link", "a":
				rewriteAttr(n, "href")
			default:
				// no-op
			}
		default:
			// no-op
		}
		for child := n.FirstChild; child != nil; child = child.NextSibling {
			rewriteNode(child)
		}
	}
	if tree, err := html.Parse(r); err != nil {
		return err
	} else {
		rewriteNode(tree)
		injectButterBar(tree)
		return html.Render(w, tree)
	}
}

// Change to working directory
func changeBinDir() {
	// read /proc/self/exe
	if dest, err := os.Readlink("/proc/self/exe"); err != nil {
		log.Printf("Error reading link: %s\n", err)
		return
	} else {
		dest = path.Dir(dest)
		log.Printf("Chdir to %s", dest)
		if err := os.Chdir(dest); err != nil {
			log.Printf("Error changing directory: %s\n", err)
		}
	}
}

// Flag page for "review"
func flagHandler(w http.ResponseWriter, r *http.Request) {
	urlString := r.PostFormValue("url")
	if !validateUrl(urlString) {
		http.Error(w, "Invalid URL provided.", http.StatusBadRequest)
		return
	}
	select {
	case reviewQueue <- urlString:
		w.Write([]byte("OK"))
	default:
		http.Error(w, "No capacity.", http.StatusInsufficientStorage)
	}
}

// Handle review by bot
func reviewHandler(w http.ResponseWriter, r *http.Request) {
	var urlString string
	select {
	case urlString = <-reviewQueue:
	default:
		w.Write([]byte("OK"))
		return
	}
	if !validateUrl(urlString) {
		http.Error(w, "Invalid URL provided.", http.StatusBadRequest)
		return
	}
	retrieveAndRewrite(w, urlString)
}

func serveStaticFile(fname string) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, fname)
	}
}

func requireAuthz(next http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		isValid := func(c *http.Cookie) bool {
			return c != nil && c.Value == getAuthzKey()
		}
		if c, err := r.Cookie("AUTHZ_KEY"); err != nil || !isValid(c) {
			http.Error(w, "Forbidden", http.StatusForbidden)
			return
		}
		next(w, r)
	}
}

func statusPage(w http.ResponseWriter, r *http.Request) {
	ipWhitelist := []string{"127.0.0.1", "127.1.1.1", "13.37.13.37", "::1"}
	isValidRemote := func(remote string) bool {
		host, _, err := net.SplitHostPort(remote)
		if err != nil {
			return false
		}
		for _, v := range ipWhitelist {
			if v == host {
				return true
			}
		}
		log.Printf("Host %s not in whitelist.", host)
		return false
	}
	if !isValidRemote(r.RemoteAddr) {
		http.Error(w, "Not in IP Whitelist", http.StatusForbidden)
		return
	}
	w.Header().Set("Content-Type", "application/json")
	respdata := make(map[string]interface{})
	respdata["ipWhitelist"] = ipWhitelist
	respdata["flag2"] = SSRFFlag
	respdata["reviewQueue"] = len(reviewQueue)
	respdata["reviewQueueSize"] = reviewQueueSize
	respdata["remoteAddr"] = r.RemoteAddr
	respdata["author"] = "Matir"
	enc := json.NewEncoder(w)
	enc.SetIndent("", "  ")
	enc.Encode(respdata)
}

func loadSSRFFlag() {
	if buf, err := ioutil.ReadFile("./flag2.txt"); err != nil {
		panic(err)
	} else {
		SSRFFlag = strings.TrimSpace(string(buf))
	}
}

func main() {
	// init steps
	reviewQueue = make(chan string, reviewQueueSize)
	changeBinDir()
	loadSSRFFlag()

	// Webserver
	fs := http.FileServer(http.Dir("./static/"))
	http.Handle("/static/", http.StripPrefix("/static/", fs))
	http.Handle("/hack", http.RedirectHandler("https://systemoverlord.com/", http.StatusMovedPermanently))
	http.HandleFunc("/render", handleRender)
	http.HandleFunc("/flag", flagHandler)
	http.HandleFunc("/review", requireAuthz(reviewHandler))
	http.HandleFunc("/status", statusPage)
	http.HandleFunc("/flag.txt", requireAuthz(serveStaticFile("flag.txt")))
	http.HandleFunc("/", serveStaticFile("index.html"))
	http.HandleFunc("/robots.txt", serveStaticFile("robots.txt"))
	port = ":8081"
	log.Printf("Starting serving on %s", port)
	log.Fatal(http.ListenAndServe(port, nil))
}
