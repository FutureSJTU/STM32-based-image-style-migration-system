from diffusers import KandinskyImg2ImgPipeline, KandinskyPriorPipeline, KandinskyPipeline
import torch

from PIL import Image
import requests
from io import BytesIO
from diffusers.utils import load_image
import PIL

from torchvision import transforms

def img2img(prompt = '', original_image=None):
    if not original_image:
        dirpath = "out.jpg"
        original_image = Image.open(dirpath)


    generator = torch.Generator()

    # create prior
    pipe_prior = KandinskyPriorPipeline.from_pretrained("kandinsky-community/kandinsky-2-1-prior", torch_dtype=torch.float16)
    pipe_prior.to("cuda")

    # create img2img pipeline
    pipe = KandinskyImg2ImgPipeline.from_pretrained("kandinsky-community/kandinsky-2-1", torch_dtype=torch.float16)
    pipe.to("cuda")

    # prompt = ""
    negative_prompt = "low quality, bad quality"

    image_emb = pipe_prior(
        prompt, guidance_scale=4.0, num_inference_steps=25, generator=generator, negative_prompt=negative_prompt
    ).image_embeds

    zero_image_emb = pipe_prior(
        negative_prompt, guidance_scale=4.0, num_inference_steps=25, generator=generator, negative_prompt=negative_prompt
    ).negative_image_embeds

    out = pipe(
        prompt,
        image=original_image,
        image_embeds=image_emb,
        negative_image_embeds=zero_image_emb,
        height=768,
        width=1024,
        num_inference_steps=500,
        strength=0.3,
    )

    out.images[0].save("gen_from_photo.png")
    return out.images[0]




if __name__ == '__main__':
    img2img()
