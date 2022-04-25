# Welcome to Remix!

- [Remix Docs](https://remix.run/docs)

## Running Server

From your terminal:

```sh
npm run dev
```

This starts your app in development mode, rebuilding assets on file changes.
It will fail if no Ardiuno is connected to the computer. (See config below in chapter "File Structure")

## File Structure
The relevant files are the following:
- app/routes/index.tsx (this is the main file for the frontend but some backend code)
- app/services/serial.server.ts (this file is responsible for calculating the motor movements and sending those to the arduino via serialport. **Make sure to change config to use right port**)
- public/models here the models to detect facial expressions, landmarks, age, ... are stored.
