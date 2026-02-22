import Poco from "commodetto/Poco";
import parseBMF from "commodetto/parseBMF";
import parseRLE from "commodetto/parseRLE";
import Battery from "embedded:sensor/Battery";

const render = new Poco(screen);

// Load a custom font from BMF resources
function getFont(name, size) {
    const font = parseBMF(new Resource(`${name}-${size}.fnt`));
    font.bitmap = parseRLE(new Resource(`${name}-${size}-alpha.bm4`));
    return font;
}

// Fonts
const timeFont = getFont("Jersey10-Regular", 56);
const dateFont = getFont("Jersey10-Regular", 24);
const smallFont = new render.Font("Gothic-Regular", 18);

// Colors
const black = render.makeColor(0, 0, 0);
const white = render.makeColor(255, 255, 255);
const green = render.makeColor(0, 170, 0);
const yellow = render.makeColor(255, 170, 0);
const red = render.makeColor(255, 0, 0);

// Day and month names
const DAYS = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"];
const MONTHS = ["Jan", "Feb", "Mar", "Apr", "May", "Jun",
                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];

// Precompute layout positions
const blockHeight = timeFont.height + dateFont.height;
const timeY = (render.height - blockHeight) / 2;
const dateY = timeY + timeFont.height;

// Store latest time for redraws triggered by battery/connection changes
let lastDate = new Date();

// Battery state
let batteryPercent = 100;

const battery = new Battery({
    onSample() {
        batteryPercent = this.sample().percent;
        drawScreen();
    }
});
batteryPercent = battery.sample().percent;

// Connection state
let isConnected = true;

function checkConnection() {
    isConnected = watch.connected.app;
    drawScreen();
}
watch.addEventListener("connected", checkConnection);
checkConnection();

function drawBatteryBar() {
    const barWidth = (render.width / 2) | 0;
    const barX = ((render.width - barWidth) / 2) | 0;
    const barY = render.height < 180 ? 6 : 20;
    const barHeight = 8;

    // Draw border
    render.fillRectangle(white, barX, barY, barWidth, barHeight);
    render.fillRectangle(black, barX + 1, barY + 1, barWidth - 2, barHeight - 2);

    // Choose color based on battery level
    let barColor;
    if (batteryPercent <= 20) {
        barColor = red;
    } else if (batteryPercent <= 40) {
        barColor = yellow;
    } else {
        barColor = green;
    }

    // Draw filled portion
    const fillWidth = ((batteryPercent * (barWidth - 4)) / 100) | 0;
    render.fillRectangle(barColor, barX + 2, barY + 2, fillWidth, barHeight - 4);
}

function drawScreen(event) {
    const now = event?.date ?? lastDate;
    if (event?.date) lastDate = event.date;

    render.begin();
    render.fillRectangle(black, 0, 0, render.width, render.height);

    // Draw battery bar at top
    drawBatteryBar();

    // Draw Bluetooth disconnect indicator below battery bar
    if (!isConnected) {
        const btStr = "X";
        const btWidth = render.getTextWidth(btStr, smallFont);
        const btY = render.height < 180 ? 16 : 30;
        render.drawText(btStr, smallFont, red,
            (render.width - btWidth) / 2, btY);
    }

    // Format time as HH:MM
    const hours = String(now.getHours()).padStart(2, "0");
    const minutes = String(now.getMinutes()).padStart(2, "0");
    const timeStr = `${hours}:${minutes}`;

    // Draw time centered
    let width = render.getTextWidth(timeStr, timeFont);
    render.drawText(timeStr, timeFont, white,
        (render.width - width) / 2, timeY);

    // Format date as "Mon Jan 01"
    const dayName = DAYS[now.getDay()];
    const monthName = MONTHS[now.getMonth()];
    const dateStr = `${dayName} ${monthName} ${String(now.getDate()).padStart(2, "0")}`;

    // Draw date centered below time
    width = render.getTextWidth(dateStr, dateFont);
    render.drawText(dateStr, dateFont, white,
        (render.width - width) / 2, dateY);

    render.end();
}

// Update every minute (fires immediately when registered)
watch.addEventListener("minutechange", drawScreen);
