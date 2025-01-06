# Disclaimer; For education pruposes only

To protect the security and integrity of the application which may still be actively used, the API `api` and application `src` has been heavily stripped with any sensitive information removed partially or in it's entirety. As a result, this project is not in a compile-ready state, and may not be readable.

As a sidenote, many portions of the codebase may not be best practice, as it was created with a very tight time constraint (1 week) to replace an existing production application which had a 40% failure rate.

## About

### API

The API is a serverless API running on Cloudflare Workers. A design limitation with the API is the lack of awareness (stateless). As a result and as a security technique to prevent tampering and hinder malicious actors, the API was designed to incrementally authorize a user to the next endpoint using JWT.

- Acts as a router to send each request to the correct endpoint
- Uses JWT for endpoint authorization
- Uses Sentry for error monitoring
- Serverless and scalable
- Streams modules to the Application

### Application

The application is a C++ application running on Windows, which authenticates to an API that streams modules to the user which unlocks further functionality. It uses IMGUI as a renderer. The API instructs the application which stages of the application to render.

- Uses advanced techniques (though this is completely stripped out, I promise it was cool)
- Tries to be as dependent on the server as possible to avoid client-side manipulation (most of this is stripped out... sorry!)