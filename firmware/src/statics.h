#ifndef _STATICS_H
#define _STATICS_H

#define DEVICE_ID "esp-iot-1"

static const char* html_setup =
R"END(
<html>

<head>
    <style>
        * {
            font-family: sans-serif;
            font-size: 15pt;
        }

        div {
            padding: 15px;
        }

        label {
            width: 100pt;
            display: inline-block;
        }

        input {
            grid-row: 5;
            padding: 5px;
            border: 1px solid #aaa;
            width: 250pt;
        }

        @media screen and (max-width: 800px) {
            * {
                font-size: 30pt;
            }

            label {
                width: 150pt;
            }
        }
    </style>
</head>

<body>
    <form action="/setup" method="POST">
        <div>
            <label for="ssid">SSID:</label>
            <input type="text" name="ssid" required />
        </div>

        <div>
            <label for="password">Password:</label>
            <input type="password" name="password" required />
        </div>

        <div>
            <label for="apikey">API Key:</label>
            <input type="text" name="apikey" required />
        </div>

        <div>
            <input type="submit" value="Complete Setup" />
        </div>
    </form>
</body>

</html>
)END";

#endif