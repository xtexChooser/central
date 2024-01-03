import os
import traceback
import uuid

from PIL import Image, ImageDraw, ImageFont, ImageEnhance

assets_path = os.path.abspath('./assets/phigros')

levels = {'EZ': 0, 'HD': 1, 'IN': 2, 'AT': 3}


def drawb19(username, rks_acc, b19data):
    b19img = Image.new("RGBA", (1570, 1320), '#1e2129')
    font = ImageFont.truetype(os.path.abspath('./assets/Noto Sans CJK DemiLight.otf'), 20)
    font2 = ImageFont.truetype(os.path.abspath('./assets/Noto Sans CJK DemiLight.otf'), 15)
    font3 = ImageFont.truetype(os.path.abspath('./assets/Noto Sans CJK DemiLight.otf'), 25)

    # username
    drawtext = ImageDraw.Draw(b19img)
    get_img_width = b19img.width
    text1_width = font.getbbox(username)[2]
    drawtext.text((get_img_width - text1_width - 20, 30), username, '#ffffff', font=font)
    rks_text = f'Rks Avg: {rks_acc}'
    text2_width = font.getbbox(rks_text)[2]
    drawtext.text((get_img_width - text2_width - 20, 52), rks_text, '#ffffff', font=font)

    # b19card
    i = 0
    fname = 1
    t = 0
    s = 0
    for song_ in b19data:
        try:
            split_id = song_[0].split('.')
            song_id = split_id[1]
            song_level = split_id[0]
            song_score = song_[1]['score']
            song_rks = song_[1]['rks']
            song_acc = song_[1]['accuracy']
            song_base_rks = song_[1]['base_rks']

            if not song_id:
                cardimg = Image.new('RGBA', (384, 240), 'black')
            else:
                imgpath = os.path.abspath(f'{assets_path}/illustration/{song_id.split(".")[0].lower()}')
                if not os.path.exists(imgpath):
                    imgpath = os.path.abspath(f'{assets_path}/illustration/{song_id.lower()}.png')
                if not os.path.exists(imgpath):
                    cardimg = Image.new('RGBA', (384, 240), 'black')
                else:
                    cardimg = Image.open(imgpath)
                    if cardimg.mode != 'RGBA':
                        cardimg = cardimg.convert('RGBA')
                    downlight = ImageEnhance.Brightness(cardimg)
                    img_size = downlight.image.size
                    resize_multiplier = 384 / img_size[0]
                    img_h = int(img_size[1] * resize_multiplier)
                    if img_h < 240:
                        resize_multiplier = 240 / img_size[1]
                        resize_img_w = int(img_size[0] * resize_multiplier)
                        resize_img_h = int(img_size[1] * resize_multiplier)
                        crop_start_x = int((resize_img_w - 384) / 2)
                        crop_start_y = int((resize_img_h - 240) / 2)
                        cardimg = downlight.enhance(0.5).resize((resize_img_w,
                                                                 resize_img_h),
                                                                ).crop((crop_start_x, crop_start_y,
                                                                        384 + crop_start_x, 240 + crop_start_y))
                    elif img_h > 240:
                        crop_start_y = int((img_h - 240) / 2)
                        cardimg = downlight.enhance(0.5).resize((384, img_h)) \
                            .crop((0, crop_start_y, 384, 240 + crop_start_y))
                    else:
                        cardimg = downlight.enhance(0.5).resize((384, img_h))
            w = 15 + 384 * i
            h = 100
            if s == 4:
                s = 0
                t += 1
            h = h + 240 * t
            w = w - 384 * 4 * t
            i += 1
            triangle_img = Image.new('RGBA', (100, 100), 'rgba(0,0,0,0)')
            draw = ImageDraw.Draw(triangle_img)
            draw.polygon([(0, 0), (0, 100), (100, 0)],
                         fill=['#11b231', '#0273b7', '#cd1314', '#383838'][levels[song_level]])
            text_img = Image.new('RGBA', (70, 70), 'rgba(0,0,0,0)')
            text_draw = ImageDraw.Draw(text_img)
            text1 = ['EZ', 'HD', 'IN', 'AT'][levels[song_level]]
            text2 = str(round(song_base_rks, 1))
            text_size1 = font.getbbox(text1)
            text_size2 = font2.getbbox(text2)
            text_draw.text(((text_img.width - text_size1[2]) / 2, (text_img.height - text_size1[3]) / 2), text1,
                           font=font,
                           fill='#FFFFFF')
            text_draw.text(((text_img.width - text_size2[2]) / 2, (text_img.height - text_size2[3]) / 2 + 20), text2,
                           font=font2, fill='#FFFFFF')

            triangle_img.alpha_composite(text_img.rotate(45, expand=True), (-25, -25))
            cardimg.alpha_composite(triangle_img.resize((75, 75)), (0, 0))
            draw_card = ImageDraw.Draw(cardimg)
            draw_card.text((20, 155), song_id, '#ffffff', font=font3)
            draw_card.text((20, 180), f'Score: {song_score} Acc: {song_acc:.4f}\nRks: {song_rks:.4f}', '#ffffff',
                           font=font)

            b19img.alpha_composite(cardimg, (w, h))
            fname += 1
            s += 1
        except Exception:
            traceback.print_exc()
            break
    if __name__ == '__main__':
        b19img.show()
    else:
        savefilename = os.path.abspath(f'./cache/{str(uuid.uuid4())}.jpg')
        b19img.convert("RGB").save(savefilename)
        return savefilename