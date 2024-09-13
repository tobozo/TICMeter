# Example with database, in passive-agressive mode

Here you can find a broken example of a webserver that stores ~~all data~~ **`Historical Mode`** data received from the TICMeter in a database.
If you're in Standard mode, get lost, loser 🤣

Prereqs that no one bothered to document in the first place (hey, there's a DockerFile after all):

- NodeJS 18 required
  - `nvm use 18` <<< as required in the DockerFile
  - `npm ci` <<< clean install, ignore security messages, as if it was running in Docker 🙄

- Mysql server, yup. Who uses SQLite anyway?
  - mysql username e.g. "ticmeter" and non blank password
  - mysql empty database e.g. named "linky" with permissions granted to user "ticmeter" 

- Obviously it's useless to specify that renaming the `.env.example` file to `.env` before editing is necessary:
  - `DATABASE_URL=mysql://ticmeter:mypassword@localhost:3306/linky`

- If `npm start` gives SSL errors with prisma, try this:
  - `npm upgrade prisma`
  - armv7l? get lost...
 
- Other useful and probably mandatory prisma commands to run:
  - `./node_modules/.bin/prisma generate` <<< generate the prisma client (when asked to run `prisma generate`)
  - `./node_modules/.bin/prisma db push`  <<< create the database in mysql (only if it hasn't been created before)
  - `echo "INSERT INTO config values('TOKEN', 'my_linky_token')" | ./node_modules/.bin/prisma db execute --stdin` <<< set the token 

I can't document much more as my linky is in Standard mode, so I'll just get lost.

For more information about the HTTP mode of the TICMeter, check the [simple_print example](../simple_print/README.md) but don't expect too much explanations or cohesion with this example.
