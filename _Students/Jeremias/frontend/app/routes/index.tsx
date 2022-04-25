import { useEffect, useRef, useState } from 'react';
import {
  ActionFunction,
  LoaderFunction,
  useFetcher,
  useTransition,
} from 'remix';
import { Tab } from '@headlessui/react';
import * as faceapi from 'face-api.js';

import faceServer from '~/services/face.server';
import sendCommand, { openPort } from '~/services/serial.server';

export const loader: LoaderFunction = async () => {
  openPort();
  return null;
};

const map = ({
  value,
  inMin,
  inMax,
  outMin,
  outMax,
}: {
  value: number;
  inMin: number;
  inMax: number;
  outMin: number;
  outMax: number;
}) => {
  return ((value - inMin) * (outMax - outMin)) / (inMax - inMin) + outMin;
};

type Detections = faceapi.WithAge<
  faceapi.WithGender<
    faceapi.WithFaceExpressions<
      faceapi.WithFaceLandmarks<
        {
          detection: faceapi.FaceDetection;
        },
        faceapi.FaceLandmarks68
      >
    >
  >
>;

export const action: ActionFunction = async ({ request }) => {
  const coords = [];
  const formData = Object.fromEntries(await request.formData());

  const detections: Detections = JSON.parse(formData.detections as string);

  // Unused Image Data
  const { r, g, b, channels } = await faceServer(formData.dataURL as string);
  const {
    age,
    landmarks,
    expressions: {
      // @ts-ignore
      array: [{ expression }],
    },
    gender,
  } = detections;

  // fill coords array with landmarks
  coords.push({ x: 52.5, y: 0 });

  for (const coord of landmarks.relativePositions) {
    const x = map({
      // @ts-ignore
      value: +coord._x,
      inMin: 0,
      inMax: 1,
      outMin: 20,
      outMax: 80,
    });
    const y = map({
      // @ts-ignore
      value: 1 - +coord._y,
      inMin: 0,
      inMax: 1,
      outMin: 20,
      outMax: 110,
    });
    coords.push({ x, y });
  }
  coords.push({ x: 52.5, y: 0 });

  // send command to arduino
  sendCommand(coords, +formData?.distortion ?? 0);

  return null;
};

const distortions = [
  '0',
  '2',
  '4',
  '6',
  '8',
  '10',
  '12',
  '14',
  '16',
  '18',
  '20',
];

export default function Index() {
  const videoRef = useRef<HTMLVideoElement>(null);
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const imgRef = useRef<HTMLImageElement>(null);
  const readPhotoRef = useRef<HTMLButtonElement>(null);
  const takePictureRef = useRef<HTMLButtonElement>(null);

  const fetcher = useFetcher();

  const transition = useTransition();

  const width = 1920;
  const [height, setHeight] = useState(0);
  const [streaming, setStreaming] = useState(false);
  const [dataURL, setDataURL] = useState('');
  const [error, setError] = useState('');
  const [distortion, setDistortion] = useState('0');

  function clearphoto() {
    if (canvasRef.current) {
      const context = canvasRef.current.getContext('2d');
      if (context) {
        context.fillStyle = '#333';
        context.fillRect(
          0,
          0,
          canvasRef.current.width,
          canvasRef.current.height
        );
      }

      const data = canvasRef.current.toDataURL('image/jpeg');
      imgRef.current?.setAttribute('src', data);
    }
  }

  function takepicture() {
    if (canvasRef.current) {
      const context = canvasRef.current.getContext('2d');
      if (context) {
        if (width && height) {
          console.log('takepicture invoked');
          canvasRef.current.width = width;
          canvasRef.current.height = height;
          if (videoRef.current) {
            context.drawImage(videoRef.current, 0, 0, width, height);
          }

          const data = canvasRef.current.toDataURL('image/jpeg');
          setDataURL(data);
          imgRef.current?.setAttribute('src', data);
        } else {
          clearphoto();
        }
      }
    }
  }

  async function readPhoto() {
    console.log('readphotoref click');
    if (imgRef.current) {
      const t = await faceapi
        .detectSingleFace(imgRef.current)
        .withFaceLandmarks()
        .withFaceExpressions()
        .withAgeAndGender();

      console.log({ t });

      if (t) {
        const detections = {
          ...t,
          expressions: {
            ...t?.expressions,
            array: t?.expressions.asSortedArray(),
          },
          landmarks: {
            ...t?.landmarks,
            relativePositions: t?.landmarks.relativePositions,
          },
        };
        fetcher.submit(
          {
            detections: JSON.stringify(detections),
            dataURL,
            distortion,
          },
          { method: 'post' }
        );
      } else {
        setError('No face detected!');
        setTimeout(() => setError(''), 3000);
      }
    }
  }

  const onKeyDown = (e: KeyboardEvent) => {
    if (e.key === 'p') {
      takePictureRef.current?.click();
    }
    if (e.key === 'd') {
      readPhotoRef.current?.click();
    }
  };

  useEffect(() => {
    (async () => {
      await faceapi.loadSsdMobilenetv1Model('/models');
      await faceapi.loadFaceExpressionModel('/models');
      await faceapi.loadFaceLandmarkModel('/models');
      await faceapi.loadAgeGenderModel('/models');
      await document.documentElement.requestFullscreen();
    })();

    navigator.mediaDevices
      .getUserMedia({ video: true, audio: false })
      .then(function (stream) {
        if (videoRef.current) {
          videoRef.current.srcObject = stream;
        }
      })
      .catch(function (error) {
        console.log('Something went wrong!' + error);
      });

    videoRef.current?.addEventListener(
      'canplay',
      function () {
        if (!streaming && videoRef.current) {
          setHeight(
            videoRef.current.videoHeight / (videoRef.current.videoWidth / width)
          );

          if (isNaN(height)) {
            setHeight(width / (16 / 9));
          }

          videoRef.current.setAttribute('width', width.toString());
          videoRef.current.setAttribute('height', height.toString());
          canvasRef.current?.setAttribute('width', width.toString());
          canvasRef.current?.setAttribute('height', height.toString());
          setStreaming(true);
        }
      },
      false
    );
    window.addEventListener('keydown', onKeyDown);
    clearphoto();
    return () => {
      window.removeEventListener('keydown', onKeyDown);
    };
  }, []);

  return (
    <div className="flex px-16 pb-16 pt-32 gap-16 h-screen items-center justify-center bg-gray-900">
      <div className="fixed inset-x-0 text-center text-3xl  top-0 z-50 font-bold h-32 grid place-items-center ">
        <h1 className="text-white flex items-center">
          {error ? (
            <span className="text-red-500">{error}</span>
          ) : (
            <div className="flex gap-2 items-center">
              Press{' '}
              <kbd className="bg-gray-700 aspect-square w-10 grid place-items-center leading-none font-medium border-gray-600 px-2 py-1 border rounded-md">
                P
              </kbd>{' '}
              to take picture <span className="mx-3">&rarr;</span> Press{' '}
              <kbd className="bg-gray-700 font-medium aspect-square w-10 grid place-items-center leading-none border-gray-600 px-2 py-1 border rounded-md">
                D
              </kbd>{' '}
              to draw
            </div>
          )}
        </h1>
      </div>

      <div className="w-1/2  h-full relative flex justify-center">
        <video
          ref={videoRef}
          autoPlay
          className="w-full h-full object-cover -scale-x-100 rounded-3xl shadow-2xl shadow-gray-500/5 border border-gray-500/20"
        />
        <button ref={takePictureRef} onClick={takepicture} className="hidden">
          Foto machen
        </button>
      </div>
      <div className="w-1/2 h-full flex justify-center relative">
        <img
          ref={imgRef}
          alt="The screen capture will appear in this box."
          className="w-full h-full object-cover -scale-x-100 rounded-3xl shadow-2xl shadow-gray-500/5  border border-gray-500/20"
        />
        <button
          disabled={!dataURL}
          onClick={readPhoto}
          ref={readPhotoRef}
          className="hidden"
        >
          {transition.state === 'submitting'
            ? 'Am hochladen...'
            : 'Foto verwenden'}
        </button>
        {dataURL ? (
          <div className="absolute bottom-4 inset-x-4 p-6 rounded-3xl bg-primary-50/50 border border-primary-900/20 backdrop-blur-xl z-50 shadow-2xl grid gap-4">
            <span className="text-lg font-bold text-primary-900">
              Distortion
            </span>
            <Tab.Group onChange={(i) => setDistortion(distortions[i])}>
              <Tab.List className="flex p-1 space-x-1 bg-primary-900/10 rounded-xl font-mono">
                {distortions.map((category) => (
                  <Tab
                    key={category}
                    className={({ selected }) =>
                      `w-full py-2.5 text-sm leading-5 font-medium rounded-lg focus:outline-none focus-visible:ring-2 ring-primary-500 ring-opacity-50 ${
                        selected
                          ? 'bg-primary-500 shadow text-white'
                          : 'text-primary-900 hover:bg-primary-900/10'
                      }`
                    }
                  >
                    {category}mm
                  </Tab>
                ))}
              </Tab.List>
            </Tab.Group>
          </div>
        ) : null}
      </div>
      <canvas className="hidden" ref={canvasRef} />
    </div>
  );
}
