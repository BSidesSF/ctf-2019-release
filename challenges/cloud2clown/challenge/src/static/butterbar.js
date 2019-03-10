(function() {
  var processedUrl = document.currentScript.getAttribute('data-base-url');
  console.log(processedUrl);
  var bar = document.createElement("div");
  var p = document.createElement("p");
  p.appendChild(document.createTextNode("Page rendered by Cloud2Clown."));
  bar.appendChild(p);
  var btn = document.createElement("input");
  btn.type = 'button';
  btn.value = 'Report Page Error';
  bar.appendChild(btn);

  // Style bar
  bar.style.all = "initial";
  bar.style.backgroundColor = "#000000";
  bar.style.color = "#aaaaaa";
  bar.style.position = "fixed";
  bar.style.zIndex = 9999;
  bar.style.textAlign = "center";
  bar.style.bottom = 0;
  bar.style.left = 0;
  bar.style.right = 0;
  bar.style.paddingTop = "10px";
  bar.style.paddingBottom = "10px";
  bar.style.fontFamily = "Helvetica, sans-serif"
  // Style button
  btn.style.color = "#1a73e8";
  btn.style.backgroundColor = "#cccccc";
  btn.style.borderRadius = "2px";
  btn.style.borderColor = "#1a73e8";
  btn.style.borderWidth = "1px";
  btn.style.padding = "3px";
  // Style p
  p.style.color = "#aaaaaa";
  p.style.display = "inline";
  p.style.marginRight = "50px";
  p.style.paddingTop = "3px";
  p.style.paddingBottom = "3px";

  // Click handler
  btn.addEventListener("click", function() {
    btn.disabled = true;
    var params = new URLSearchParams();
    var fail = function() {
      btn.value = 'Submit Failed!';
    };
    params.append("url", processedUrl);
    fetch("/flag", {
      method: "POST",
      body: params,
    }).then(function(r) {
      if (r.ok) {
        btn.value = 'Submitted!';
        return
      }
      fail();
    }).catch(function (e) {
      console.log(e);
      fail();
    })
  });

  if (document.body)
    document.body.appendChild(bar);
}());
