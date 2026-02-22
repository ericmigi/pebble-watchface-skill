import Poco from "commodetto/Poco";

const render = new Poco(screen);
const background = render.makeColor(0, 0, 0);
const foreground = render.makeColor(255, 255, 255);
const font = new render.Font("Gothic-Bold", 24);

function draw(event) {
  const now = event.date;
  const hh = String(now.getHours()).padStart(2, "0");
  const mm = String(now.getMinutes()).padStart(2, "0");
  const text = `${hh}:${mm}`;

  render.begin();
  render.fillRectangle(background, 0, 0, render.width, render.height);
  const width = render.getTextWidth(text, font);
  render.drawText(text, font, foreground, (render.width - width) / 2, (render.height - font.height) / 2);
  render.end();
}

watch.addEventListener("minutechange", draw);
